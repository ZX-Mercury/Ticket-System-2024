#include <iostream>
#include <string>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <vector>
#include <set>
#include "BPT.h"

struct user {
    char username[25];
    char password[35];
    char name[18];//真实姓名，2~5个汉字，一个汉字的 UTF-8 编码为3字节。
    char mailAddr[35];//仅含数字、大小写字母，@ 和 .。
    int privilege;//所处用户组优先级，为 0~10 中的一个整数。
    bool operator<(const user &rhs) const {
        return strcmp(username, rhs.username) < 0;
    }

    bool operator==(const user &rhs) const {
        return strcmp(username, rhs.username) == 0;
    }

    bool operator>(const user &rhs) const {
        return rhs < *this;
    }
};

BPTree<user> user_db;
std::set<std::string> online_users;//在线用户
struct train {
    char trainID[25];       //字母、数字和下划线组成的字符串，长度不超过 20。
    int stationNum;         //车次经过的车站数量，一个不低于 2 且不超过 100 的整数。
    char stations[105][35]; //车次经过的所有车站名，共 stationNum 项，每个站名由汉字组成，不超过 10 个汉字。
    int seatNum;            //该车次的座位数，一个不超过 $10^5$ 的整数。
    int prices[100];        //每两站之间的票价，共 (stationNum - 1) 项，第 i 项表示第 i 站到第 (i+1) 站的票价，为一个不超过 $10^5$ 的整数。
    int startTime;          //（转为分钟制）列车每日的发车时间。时间格式为 hh:mm ，例如 23:51
    int travelTimes[105];   //每两站之间行车所用的时间，共 (stationNum - 1) 项。单位：分钟。每一项是一个不超过 $10^4$ 的整数。
    int stopoverTimes[105]; //除始发站和终点站之外，列车在每一站停留的时间，共 (stationNum - 2) 项。单位：分钟。每一项为一个不超过 $10^4$ 的整数。
    int saleDate;           //车次的售卖时间区间。由 2 项时间表示，每一项均为 2024 年 6 月至 8 月的某一日期。
    //日期格式为 mm-dd，例如：06-01、08-31
    char type;              //列车类型，一个大写字母。
};

std::vector<std::string> split(std::string command) {
    std::vector<std::string> tokens;
    std::string token;
    for (int i = 0; i < command.size(); i++) {
        if (command[i] == ' ') {
            tokens.push_back(token);
            token = "";
        } else {
            token += command[i];
        }
    }
    tokens.push_back(token);
    return tokens;
}

const int month_days[13] = {0, 31, 59, 90, 120, 151,
                            181, 212, 243, 273, 304, 334, 365};

int stod(const char str[25]) {//将mm-dd转为一个整数
    int date;
    int month, day;
    month = (str[0] - '0') * 10 + str[1] - '0';
    day = (str[3] - '0') * 10 + str[4] - '0';
    date = month_days[month - 1] + day;
    return date;
}

bool add_user(user tmp) {
    if (!user_db.query(tmp.username).empty()) return false;
    user_db.insert(tmp.username, tmp);
    return true;
}

bool check_add(char cur_username[25], int privilege) {
    std::vector<user> t = user_db.query(cur_username);
    if (t.empty()) return false;
    if (t.size() != 1) throw ("Duplicate username");
    if (t[0].privilege < privilege) return false;
    return true;
}

bool login(char cur_username[25], char cur_password[35]) {
    std::vector<user> t = user_db.query(cur_username);
    if (t.empty()) return false;//用户不存在
    if (t.size() != 1) throw ("Duplicate username");
    if (strcmp(t[0].password, cur_password) != 0) return false;//密码错误
    if (online_users.find(cur_username) != online_users.end()) return false;//用户已登录
    online_users.insert(cur_username);
    return true;
}

bool logout(char cur_username[25]) {
    if (online_users.find(cur_username) == online_users.end()) return false;//用户未登录
    online_users.erase(cur_username);
    return true;
}

void query_profile(char cur_username[25], char username[35]) {
    if (online_users.find(cur_username) == online_users.end()) {//用户未登录
        std::cout << "-1" << std::endl;
        return;
    }
    auto tv = user_db.query(username);
    if (tv.empty()) {//用户不存在
        std::cout << "-1" << std::endl;
        return;
    }
    user t = tv[0];
    if (user_db.query(cur_username)[0].privilege <= user_db.query(username)[0].privilege
        || strcmp(cur_username, username) != 0) {//权限不足或不是自己
        std::cout << "-1" << std::endl;
        return;
    }
    std::cout << t.username << ' ' << t.name << ' ' << t.mailAddr << ' ' << t.privilege << std::endl;
}
//-c 已登录，且「-c 的权限大于 -u 的权限，或是 -c 和 -u 相同」，且 -g 需低于 -c 的权限。
bool check_modify_profile(char cur_username[25], char new_username[25], int new_privilege = -1) {
    if (online_users.find(cur_username) == online_users.end()) return false;//用户未登录
    if (user_db.query(cur_username)[0].privilege <= user_db.query(new_username)[0].privilege
        || strcmp(cur_username, new_username) != 0) return false;//权限不足或不是自己
    if (new_privilege != -1 && new_privilege >= user_db.query(cur_username)[0].privilege) return false;
    return true;
}
//用户-c(<cur_username>) 修改 -u(<username>) 的用户信息。修改参数同注册参数，且均可以省略。
void modify_profile(char cur_username[25], char new_username[25], char new_password[35] = "", char new_name[18] = "",
                    char new_mailAddr[35] = "", int new_privilege = -1) {
    user t = user_db.query(new_username)[0];
    if (new_password != "") strcpy(t.password, new_password);
    if (new_name != "") strcpy(t.name, new_name);
    if (new_mailAddr != "") strcpy(t.mailAddr, new_mailAddr);
    if (new_privilege != -1) t.privilege = new_privilege;

}

bool check_delete_train(char trainID[25]) {}

void delete_train(char trainID[25]) {}

bool check_release_train(char trainID[25]) {}

void release_train(char trainID[25]) {}


int main() {
    while (true) {
        std::string command;
        getline(std::cin, command);
        std::vector<std::string> tokens = split(command);
        std::cout << tokens[0] << ' ';
        if (tokens[1] == "add_user") {
            char cur_username[25];
            user new_user;
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-c") strcpy(cur_username, tokens[i + 1].c_str());
                else if (tokens[i] == "-u") strcpy(new_user.username, tokens[i + 1].c_str());
                else if (tokens[i] == "-p") strcpy(new_user.password, tokens[i + 1].c_str());
                else if (tokens[i] == "-n") strcpy(new_user.name, tokens[i + 1].c_str());
                else if (tokens[i] == "-m") strcpy(new_user.mailAddr, tokens[i + 1].c_str());
                else if (tokens[i] == "-g") new_user.privilege = std::stoi(tokens[i + 1]);
            }
            if (!check_add(cur_username, new_user.privilege)) {
                std::cout << "0" << std::endl;
                continue;
            }
            if (!add_user(new_user)) {
                std::cout << "0" << std::endl;
                continue;
            }
            std::cout << "1" << std::endl;
        } else if (tokens[1] == "login") {
            char cur_username[25], cur_password[35];
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-u") strcpy(cur_username, tokens[i + 1].c_str());
                else if (tokens[i] == "-p") strcpy(cur_password, tokens[i + 1].c_str());
            }
            if (!login(cur_username, cur_password)) {
                std::cout << "0" << std::endl;
                continue;
            }
            std::cout << "1" << std::endl;
        } else if (tokens[1] == "logout") {
            char cur_username[25];
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-c") strcpy(cur_username, tokens[i + 1].c_str());
            }
            if (!logout(cur_username)) {
                std::cout << "0" << std::endl;
                continue;
            }
            std::cout << "1" << std::endl;
        } else if (tokens[1] == "query_profile") {
            char cur_username[25], username[35];
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-c") strcpy(cur_username, tokens[i + 1].c_str());
                else if (tokens[i] == "-u") strcpy(username, tokens[i + 1].c_str());
            }
            query_profile(cur_username, username);
        } else if (tokens[1] == "modify_profile") {
            char cur_username[25], new_username[25], new_password[35], new_name[18], new_mailAddr[35];
            int new_privilege;
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-c") strcpy(cur_username, tokens[i + 1].c_str());
                else if (tokens[i] == "-u") strcpy(new_username, tokens[i + 1].c_str());
                    //下面的是可以缺省的内容
                else if (tokens[i] == "-p") strcpy(new_password, tokens[i + 1].c_str());
                else if (tokens[i] == "-n") strcpy(new_name, tokens[i + 1].c_str());
                else if (tokens[i] == "-m") strcpy(new_mailAddr, tokens[i + 1].c_str());
                else if (tokens[i] == "-g") new_privilege = std::stoi(tokens[i + 1]);
            }
            if (!check_modify_profile(cur_username, new_username, new_privilege)) {
                std::cout << "-1" << std::endl;
                continue;
            }
            modify_profile(cur_username, new_username, new_password, new_name, new_mailAddr, new_privilege);
        } else if (tokens[1] == "add_train") {
            train new_train;
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-i") strcpy(new_train.trainID, tokens[i + 1].c_str());
                else if (tokens[i] == "-n") new_train.stationNum = std::stoi(tokens[i + 1]);
                else if (tokens[i] == "-m") new_train.seatNum = std::stoi(tokens[i + 1]);
                else if (tokens[i] == "-s") {
                    for (int j = 0; j < new_train.stationNum; j++) {
                        strcpy(new_train.stations[j], tokens[i + 1].c_str());
                    }
                } else if (tokens[i] == "-p") {
                    for (int j = 0; j < new_train.stationNum - 1; j++) {
                        new_train.prices[j] = std::stoi(tokens[i + 1]);
                    }
                } else if (tokens[i] == "-x") new_train.startTime = std::stoi(tokens[i + 1]);
                else if (tokens[i] == "-t") {
                    for (int j = 0; j < new_train.stationNum - 1; j++) {
                        new_train.travelTimes[j] = std::stoi(tokens[i + 1]);
                    }
                } else if (tokens[i] == "-o") {
                    for (int j = 0; j < new_train.stationNum - 2; j++) {
                        new_train.stopoverTimes[j] = std::stoi(tokens[i + 1]);
                    }
                } else if (tokens[i] == "-d") new_train.saleDate = std::stoi(tokens[i + 1]);
                else if (tokens[i] == "-y") new_train.type = tokens[i + 1][0];
            }
        } else if (tokens[1] == "delete_train") {
            char trainID[25];
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-i") strcpy(trainID, tokens[i + 1].c_str());
            }
            if (!check_delete_train(trainID)) {
                std::cout << "-1" << std::endl;
                continue;
            }
            delete_train(trainID);
            std::cout << "0" << std::endl;
        } else if (tokens[1] == "release_train") {
            char trainID[25];
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-i") strcpy(trainID, tokens[i + 1].c_str());
            }
            if (!check_release_train(trainID)) {
                std::cout << "-1" << std::endl;
                continue;
            }
            release_train(trainID);
            std::cout << "0" << std::endl;
        } else if (tokens[1] == "query_train") {
            char trainID[25];
            int date;
            for (int i = 2; i < tokens.size(); i += 2) {
                if (tokens[i] == "-i") strcpy(trainID, tokens[i + 1].c_str());
                else if (tokens[i] == "-d") date = stod(tokens[i + 1]);
            }
        } else if (tokens[1] == "query_ticket") {

        } else if (tokens[1] == "query_transfer") {

        } else if (tokens[1] == "buy_ticket") {

        } else if (tokens[1] == "query_order") {

        } else if (tokens[1] == "refund_ticket") {

        } else if (tokens[1] == "clean") {

        } else if (tokens[1] == "exit") {
            std::cout << "bye" << std::endl;
            break;
        } else throw ("Invalid command");
    }
}
