#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <windows.h>
#include <sstream>
#include <iomanip>

void setcolor(int c) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, c);
}

std::string runcmd(const char* cmd) {
    char buf[128];
    std::string out;
    FILE* p = _popen(cmd, "r");
    if (!p) return "ERROR";
    while (fgets(buf, 128, p)) {
        out += buf;
    }
    _pclose(p);
    return out;
}

std::vector<std::string> adapters() {
    std::vector<std::string> list;
    std::string cmd = "powershell -Command \"Get-NetAdapter | Where-Object {$_.Status -eq 'Up'} | Select-Object -Property Name | Format-Table -HideTableHeaders\"";
    std::string res = runcmd(cmd.c_str());
    std::string tmp;
    for (char c : res) {
        if (c == '\n' || c == '\r') {
            if (!tmp.empty() && tmp.find("-----") == std::string::npos) {
                while (!tmp.empty() && tmp[0] == ' ') tmp.erase(0, 1);
                while (!tmp.empty() && tmp.back() == ' ') tmp.pop_back();
                if (!tmp.empty()) list.push_back(tmp);
            }
            tmp.clear();
        }
        else tmp += c;
    }
    return list;
}

std::string macgen() {
    std::random_device r;
    std::mt19937 g(r());
    std::uniform_int_distribution<> d(0, 255);
    std::stringstream s;
    s << std::hex << std::setfill('0');
    int fb = (d(g) & 0xFC) | 0x02;
    s << std::setw(2) << fb;
    for (int i = 0; i < 5; i++) {
        s << ":" << std::setw(2) << d(g);
    }
    std::string m = s.str();
    for (char& c : m) c = toupper(c);
    return m;
}

void changemac() {
    std::vector<std::string> adp = adapters();
    if (adp.empty()) return;

    for (auto x : adp) {
        std::cout << x << "\n";
        std::string newmac = macgen();
        std::string command = "powershell -Command \"Set-NetAdapter -Name '" + x + "' -MacAddress '" + newmac + "' -Confirm:$false\"";
        int r = system(command.c_str());
        if (r == 0) {
            setcolor(10);
            std::cout << "[success!] New MAC address: " << newmac << "\n";
            setcolor(7);
        }
        else {
            std::cout << "[failed] Error: " << r << "\n";
        }
    }
}

int main() {
    changemac();
    std::cout << "\nPress enter to finish...";
    std::cin.get();
    return 0;
}