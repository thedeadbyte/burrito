#include "argparse.hpp"
#include <Windows.h>
#include <TlHelp32.h>

class Burrito {
private:
    struct BurritoProcess {
        std::string process_name{}, parent_process_name{};
        std::size_t pid{}, parent_pid{};
    };

public:
    std::vector<BurritoProcess> process_list{};

    void get_processes(bool save_to_cache = false) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 lppe{ .dwSize = sizeof(PROCESSENTRY32) };

        if (!Process32First(hSnapshot, &lppe)) {
            throw std::exception("Burrito failed to retrieve the process list");
        }

        do {
            if (save_to_cache) {
                BurritoProcess bp{
                    .process_name = static_cast<std::string>(lppe.szExeFile),
                    .pid = lppe.th32ProcessID
                };

                process_list.push_back(bp);
            }
            else {
                std::cout << "Process name: " << static_cast<std::string>(lppe.szExeFile) << '\n';
            }
        } while (Process32Next(hSnapshot, &lppe));
    }

    void filter_processes(std::vector<std::string>& filters) {
        get_processes(true);

        for (BurritoProcess bp : process_list) {
            for (auto filter : filters) {
                if (filter == bp.process_name) {
                    std::cout << "[ Filtered ] " << bp.process_name << " [PID: " << bp.pid << "\t PPID: " << bp.parent_pid << "]" << '\n';
                }
           }
        }
    }

    void dump_process(std::string& _pid) {
        get_processes(true);
        std::size_t pid = std::stoi(_pid.c_str());

        BurritoProcess cbp{};

        for (BurritoProcess& bp : process_list) {
            if (pid == bp.pid) {
                // std::cout << "[ Searching ] " << pid << " == " << bp.pid << '\n';
                cbp = bp;
            }
        }

        if (cbp.process_name.length() == 0) {
            std::cerr << "[ Error ] Burrito cannot found the given process `" << pid << "`\n";
            std::exit(1);
        }

        std::cout << "[ Info ] Burrito found the given process: " << cbp.process_name << '\n';

        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, cbp.pid);
        LPSTR lpImageFileName{};

        // TODO

        std::exit(1);
    }
};


int main(int argc, char** argv) {
    argparse::ArgumentParser program{ "Burrito", "1.0.0" };
    program.add_description("Burrito is a lightweight command-line process explorer");

    program.add_argument("-l", "--list-all")
        .help("Show a minimalist process list")
        .default_value(false)
        .implicit_value(false);

    program.add_argument("-s", "--search-by-name")
        .help("Search a process by its name")
        .metavar("PROCESS_NAME");

    program.add_argument("-f", "--filter-by-names")
        .help("Filter one or more processes by its name")
        .metavar("PROCESS_NAME, ")
        .nargs(argparse::nargs_pattern::at_least_one);

    program.add_argument("-d", "--dump-process")
        .help("Dump the process binary content using hexadecimal")
        .nargs(1)
        .metavar("PID");

    try {
        program.parse_args(argc, argv);
        
    } catch (const std::exception& err) {
        std::cerr << "[ Error ] " << err.what() << '\n';
        std::cerr << program << '\n';
        return 1;
    }

    try {
        Burrito bmgr;
        if (program["--list-all"] == true) {
            bmgr.get_processes();
        }

        auto filters = program.get<std::vector<std::string>>("--filter-by-names");
        if (!filters.empty()) {
            bmgr.filter_processes(filters);
        }

        auto dump_process = program.get<std::vector<std::string>>("--dump-process");
        if (!dump_process.empty()) {
            bmgr.dump_process(dump_process[0]);
        }
    }
    catch (std::exception& err) {
        std::cout << "[ Error ] " << err.what() << '\n';
        std::exit(1);
    }

    /*
    std::cout <<
        "     ____                  _ __\n"\
        "    / __ )__  ____________(_) /_____\n"\
        "   / __  / / / / ___/ ___/ / __/ __ \\\n"\
        "  / /_/ / /_/ / /  / /  / / /_/ /_/ /\n"\
        " /_____/\\__,_/_/  /_/  /_/\\__/\\____/\n\n";
    std::cout << program << '\n';
    */

    return 1;
}
