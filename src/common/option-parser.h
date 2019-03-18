#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>

class OptionParser {
public:
    OptionParser() : max_arg_len_(8) {}

    bool ParseOptions(int argc, char *argv[]) {
        options_.clear();
        // argv[0] binary path, set binary name
        std::string binary_path = argv[0];
        size_t index = binary_path.rfind('/');
        binary_name_ = binary_path.substr(index != std::string::npos ? index + 1 : 0);
        // set options_
        for (int i=1; i<argc; i++){
            ParseItem(argv[i]);
        }
        // check required_
        if (!CheckRequired()) {
            Usage();
            return false;
        }
        return true;
    }

    std::string GetOption(const std::string& opt) {
        std::map<std::string, std::string>::const_iterator it = options_.find(opt);
        return it == options_.end() ? "" : it->second;
    }

    void Register(const std::string& opt, const std::string& instructions, bool is_required_ = true) {
        if (opt.length() > max_arg_len_) max_arg_len_ = (opt.length() + 7)/8*8;
        if (is_required_) {
            required_.push_back(std::make_pair(opt, instructions));
        } else {
            optional_.push_back(std::make_pair(opt, instructions));
        }
    }

    void ListOptions() {
        std::cout << "Args [" << options_.size() << "]:" << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = options_.begin(); it != options_.end(); it ++) {
            std::cout << std::left << std::setw(max_arg_len_) << it->first << ": " << it->second << std::endl;
        }
        std::cout << std::endl;
    }

private:
    int max_arg_len_;
    std::string binary_name_;
    std::map<std::string, std::string> options_;
    std::vector<std::pair<std::string, std::string>> required_;
    std::vector<std::pair<std::string, std::string>> optional_;

private:
    bool CheckRequired() {
        for (std::vector<std::pair<std::string, std::string>>::iterator it=required_.begin(); it!=required_.end(); it++) {
            const std::string& opt = it->first;
            if (options_.find(opt) == options_.end()) return false;
        }
        return true;
    }

    void Usage() {
        std::cout << "Usage: " << binary_name_ << " [OPTION]" << std::endl;
        std::cout << std::endl;
        if (required_.size()) {
            std::cout << " Required:" << std::endl << std::endl;
            for (std::vector<std::pair<std::string, std::string>>::iterator it=required_.begin(); it!=required_.end(); it++) {
                std::cout << "  --" << std::left << std::setw(max_arg_len_) << it->first << it->second << std::endl;
            }
            std::cout << std::endl;
        }
        if (optional_.size()) {
            std::cout << " Optional:" << std::endl << std::endl;
            for (std::vector<std::pair<std::string, std::string>>::iterator it=optional_.begin(); it!=optional_.end(); it++) {
                std::cout << "  --" << std::left << std::setw(max_arg_len_) << it->first << it->second << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void ParseItem(const char *str){
        if (!str || !*str) return;
        if (*str!='-' || str[1]!='-') return;
        const char *r = str+2, *key = r, *key_end = nullptr, *val = nullptr;
        while (*r) {
            if (*r=='=') {
                key_end = r++;
                val = r; break;
            }
            r++;
        }
        if (nullptr == key_end || nullptr == val) return;
        if (key==key_end || !*val) return;
        options_[std::string(key, (key_end-key))] = val;
    }
};

