#ifndef DATABASE
#define DATABASE

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <utility>

#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

template<class T, int info_len = 2>//info_len是存储文件头部预留出来的int的个数（1_base）。
class MemoryRiver {
private:
    /* your code here */
    fstream file;
    string file_name;
    int sizeofT = sizeof(T);
public:
public:
    MemoryRiver() = default;

    MemoryRiver(string file_name) : file_name(std::move(file_name)) {}

    void initialise(const string &FN = "") {
        if (!FN.empty())
            file_name = FN;
        file.open(file_name);
        if (!file) {
            file.open(file_name, std::ios::out);
            file.close();
            file.open(file_name, std::ios::in | std::ios::out);
            int tmp = -1, tmp2 = 0;
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
            for (int i = 1; i < info_len; ++i)
                file.write(reinterpret_cast<char *>(&tmp2), sizeof(int));
            //file.close();
        } else {
            //file.open(file_name, std::ios::in | std::ios::out);
        }
    }

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        //file.open(file_name, std::ios::in);
        file.seekg((n - 1) * sizeof(int), std::ios::beg);
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
        //file.close();
    }

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        //file.open(file_name, std::ios::in | std::ios::out);
        file.seekp((n - 1) * sizeof(int), std::ios::beg);
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        //file.close();
    }

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    //位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
    //位置索引index可以取为对象写入的起始位置
    int write(T &t) {
        //file.open(file_name, std::ios::in | std::ios::out);
        file.seekp(0, std::ios::end);//定位到文件末尾
        int index = file.tellp();//获取当前位置
        //file.close();//关闭文件
        index = (index - info_len * sizeof(int)) / sizeofT;
        update(t, index);//更新位置索引index对应的对象
        return index;
    }

    int getindex() {
        //file.open(file_name, std::ios::in | std::ios::out);
        file.seekp(0, std::ios::end);//定位到文件末尾
        int index = file.tellp();//获取当前位置
        //file.close();//关闭文件
        index = (index - info_len * sizeof(int)) / sizeofT;
        return index;
    }

    //用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
    void update(T &t, const int index) {
        //file.open(file_name, std::ios::in | std::ios::out);
        file.seekp(info_len * sizeof(int) + index * sizeofT);
        file.write(reinterpret_cast<char *>(&t), sizeofT);//写入t
        //file.close();
    }

    //读出位置索引index对应的T对象的值并赋值给t
    void read(T &t, const int index) {
        //file.open(file_name, std::ios::in);
        //file.seekg(index);//定位到index
        file.seekg(info_len * sizeof(int) + index * sizeofT);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
        //file.close();
    }

    //删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
    void Delete(int index) {
        /*file.open(file_name, std::ios::in | std::ios ::out);
        file.seekp(++recycle_num * sizeof(int));
        file.write(reinterpret_cast<char *> (&index), sizeof(int));
        file.seekp(0);
        file.write(reinterpret_cast<char *> (&recycle_num), sizeof(int));
        file.close();*/
    }
    void end(){
        file.close();
    }
};

#endif //BPT_MEMORYRIVER_HPP


const int MAX = 104;//105
const int MIN = MAX / 2;

template<class T>
struct Key_Value {
    char index[65];
    T value;

    Key_Value() = default;

    Key_Value(char index[65], T value) {
        std::strcpy(this->index, index);
        this->value = value;
    }

    Key_Value(const Key_Value &kv) {
        std::strcpy(index, kv.index);
        value = kv.value;
    }

    bool operator==(const Key_Value &kv) const {
        return std::strcmp(index, kv.index) == 0 && value == kv.value;
    }

    bool operator!=(const Key_Value &kv) const {
        return !(*this == kv);
    }

    bool operator<(const Key_Value &kv) const {
        if (std::strcmp(index, kv.index) != 0)
            return std::strcmp(index, kv.index) < 0;
        return value < kv.value;
    }

    bool operator<=(const Key_Value &kv) const {
        return *this < kv || *this == kv;
    }

    bool operator>(const Key_Value &kv) const {
        if (std::strcmp(index, kv.index) != 0)
            return std::strcmp(index, kv.index) > 0;
        return value > kv.value;
    }

    bool operator>=(const Key_Value &kv) const {
        return *this > kv || *this == kv;
    }
};

//template<class T>


template<class T>
class BPTree {
    class node {
        //int id;    //节点的id
        bool Is_leaf;    //是否是叶子节点
        int size;    //节点中关键字的个数
        Key_Value<T> key_value[MAX + 2];    //关键字
        int ptr[MAX + 2];  //指向子节点的指针
        int brother;   //指向兄弟节点的指针
        friend class BPTree;

    public:
        node() : key_value(), ptr(), brother(0) {//, parent(0)
            for (int i = 0; i < MAX + 1; i++) {
                ptr[i] = 0;
            }
        }
    };
public:
    int root;
private:
    MemoryRiver<node> MR;
    Key_Value<T> pass;

    bool insertInternal(node cursor, int pos, Key_Value<T> kv) {//插入（内部节点）
        // 注意比最后一个元素大要不要特判
        if (cursor.Is_leaf) {
            //二分法找到插入位置
            int l = 0, r = cursor.size;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (cursor.key_value[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }

            //如果有了
            if (l > 0 && cursor.key_value[l - 1] == kv)
                return false;

            // 插在 l 处
            if (cursor.size < MAX) {//有插入的空间
                for (int i = cursor.size - 1; i >= l; --i) {
                    cursor.key_value[i + 1] = cursor.key_value[i];
                }
                cursor.size++;
                cursor.key_value[l] = kv;
                MR.update(cursor, pos);
                return false;
            }

            //没有插入的空间，就分裂
            for (int i = cursor.size - 1; i >= l; --i) {
                cursor.key_value[i + 1] = cursor.key_value[i];
            }
            cursor.size++;
            cursor.key_value[l] = kv;
            int newpos = MR.getindex();//getindex的作用是返回最后一个位置的下一个位置

            static node newbro;
            newbro.Is_leaf = true;
            newbro.size = MIN + 1;
            newbro.brother = cursor.brother;
            cursor.brother = newpos;  // 插入到当前节点右侧
            for (int i = 0; i <= MIN; ++i) {
                newbro.key_value[i] = cursor.key_value[i + MIN];
            }
            //newbro.parent = cursor.parent;

            cursor.size = MIN;
            if (root == pos) {//如果是根节点，麻烦一点，新建一个根节点

                static node newroot;
                newroot.Is_leaf = false;
                newroot.size = 1;
                newroot.key_value[0] = cursor.key_value[MIN];
                newroot.ptr[0] = pos;
                newroot.ptr[1] = newpos;

                MR.update(cursor, pos);
                MR.update(newbro, newpos);
                int rootpos = MR.getindex();
                MR.write(newroot);
                root = rootpos;
                return false;
            }
            MR.update(cursor, pos);
            MR.update(newbro, newpos);
            pass = newbro.key_value[0];
            return true;
        }

        //-----------------------------------


        int l = 0, r = cursor.size;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (cursor.key_value[mid] >= kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        if (l < cursor.size && cursor.key_value[l] == kv) {
            ++l;
        }

        node child;
        MR.read(child, cursor.ptr[l]);

        bool state = insertInternal(child, cursor.ptr[l], kv);
        if (!state)
            return false;
        if (cursor.size < MAX) {
            for (int i = cursor.size - 1; i >= l; --i) {
                cursor.key_value[i + 1] = cursor.key_value[i];
                cursor.ptr[i + 2] = cursor.ptr[i + 1];
            }
            cursor.size++;
            cursor.key_value[l] = pass;
            cursor.ptr[l + 1] = MR.getindex()-1;
            MR.update(cursor, pos);
            return false;
        }
        for (int i = cursor.size - 1; i >= l; --i) {
            cursor.key_value[i + 1] = cursor.key_value[i];
            cursor.ptr[i + 2] = cursor.ptr[i + 1];
        }
        ++cursor.size;
        cursor.key_value[l] = pass;
        cursor.ptr[l + 1] = MR.getindex()-1;

        int newpos = MR.getindex();
        pass = cursor.key_value[MIN];
        static node newbro;
        newbro.Is_leaf = false;
        newbro.size = MIN;
        for (int i = 0; i < MIN; ++i) {
            newbro.key_value[i] = cursor.key_value[i + MIN + 1];
            newbro.ptr[i] = cursor.ptr[i + MIN + 1];
        }
        newbro.ptr[MIN] = cursor.ptr[cursor.size];

        cursor.size = MIN;
        if (root == pos) {
            // 裂根，建立新的根节点
            static node newroot;
            newroot.Is_leaf = false;
            newroot.size = 1;
            newroot.key_value[0] = pass;
            newroot.ptr[0] = pos;
            newroot.ptr[1] = newpos;
            MR.update(cursor, pos);
            MR.update(newbro, newpos);
            root = MR.write(newroot);
            return false;
        }
        MR.update(cursor, pos);
        MR.update(newbro, newpos);
        return true;
    }

    bool deleteInternal(node &cursor, int pos, const Key_Value<T> &kv) {
        if (cursor.Is_leaf) {
            int l = 0, r = cursor.size;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (cursor.key_value[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }
            --l;
            if (l < 0 || l >= cursor.size || cursor.key_value[l] != kv) {
                return false;
            }
            for (int i = l + 1; i < cursor.size; ++i) {
                cursor.key_value[i - 1] = cursor.key_value[i];
            }
            --cursor.size;
            if (pos == root) {
                MR.update(cursor, pos);
            }
            MR.update(cursor, pos);
            if (cursor.size < MIN) {
                return true;
            }
            return false;
        }

        int l = 0, r = cursor.size;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (cursor.key_value[mid] >= kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        if (l < cursor.size && kv == cursor.key_value[l]) {
            ++l;
        }
        node child;
        MR.read(child, cursor.ptr[l]);
        bool state = deleteInternal(child, cursor.ptr[l], kv);
        if (!state)
            return false;
        if (pos == root && cursor.size == 1) {
            static node newbro[2];
            MR.read(newbro[0], cursor.ptr[0]);
            MR.read(newbro[1], cursor.ptr[1]);
            if (newbro[0].size + newbro[1].size < MAX) {
                MR.read(newbro[0], cursor.ptr[0]);
                MR.read(newbro[1], cursor.ptr[1]);
                if (newbro[0].Is_leaf) {
                    for (int i = 0; i < newbro[1].size; ++i) {
                        newbro[0].key_value[i + newbro[0].size] = newbro[1].key_value[i];
                    }
                    newbro[0].size += newbro[1].size;
                    newbro[0].brother = newbro[1].brother;
                    root = cursor.ptr[0];
                    MR.update(newbro[0], cursor.ptr[0]);
                    return false;
                }
                for (int i = 0; i < newbro[1].size; ++i) {
                    newbro[0].key_value[i + newbro[0].size + 1] = newbro[1].key_value[i];
                    newbro[0].ptr[i + newbro[0].size + 1] = newbro[1].ptr[i];
                }
                newbro[0].ptr[newbro[0].size + newbro[1].size + 1] = newbro[1].ptr[newbro[1].size];
                newbro[0].key_value[newbro[0].size] = cursor.key_value[0];
                newbro[0].size += newbro[1].size + 1;
                root = cursor.ptr[0];
                MR.update(newbro[0], cursor.ptr[0]);
                return false;
            }
        }
        if (l > 0) {
            static node newbro;
            MR.read(newbro, cursor.ptr[l - 1]);
            if (newbro.size > MIN) {
                if (child.Is_leaf) {
                    MR.read(newbro, cursor.ptr[l - 1]);
                    for (int i = child.size - 1; i >= 0; --i) {
                        child.key_value[i + 1] = child.key_value[i];
                    }
                    child.key_value[0] = newbro.key_value[newbro.size - 1];
                    ++child.size;
                    --newbro.size;
                    cursor.key_value[l - 1] = child.key_value[0];
                    MR.update(cursor, pos);
                    MR.update(newbro, cursor.ptr[l - 1]);
                    MR.update(child, cursor.ptr[l]);
                    return false;
                }
                MR.read(newbro, cursor.ptr[l - 1]);
                for (int i = child.size; i >= 1; --i) {
                    child.key_value[i] = child.key_value[i - 1];
                    child.ptr[i + 1] = child.ptr[i];
                }
                child.ptr[1] = child.ptr[0];
                ++child.size;
                child.key_value[0] = cursor.key_value[l - 1];
                child.ptr[0] = newbro.ptr[newbro.size];
                cursor.key_value[l - 1] = newbro.key_value[newbro.size - 1];
                --newbro.size;
                MR.update(cursor, pos);
                MR.update(newbro, cursor.ptr[l - 1]);
                MR.update(child, cursor.ptr[l]);
                return false;
            }
            if (child.Is_leaf) {
                MR.read(newbro, cursor.ptr[l - 1]);
                for (int i = 0; i < child.size; ++i) {
                    newbro.key_value[i + newbro.size] = child.key_value[i];
                }
                newbro.size += child.size;
                newbro.brother = child.brother;
                for (int i = l; i < cursor.size; ++i) {
                    cursor.key_value[i - 1] = cursor.key_value[i];
                    cursor.ptr[i] = cursor.ptr[i + 1];
                }
                --cursor.size;
                newbro.brother = child.brother;
                MR.update(cursor, pos);
                MR.update(newbro, cursor.ptr[l - 1]);
                if (cursor.size < MIN)
                    return true;
                return false;
            }
            MR.read(newbro, cursor.ptr[l - 1]);
            for (int i = 0; i < child.size; ++i) {
                newbro.key_value[i + newbro.size + 1] = child.key_value[i];
                newbro.ptr[i + newbro.size + 1] = child.ptr[i];
            }
            newbro.ptr[newbro.size + child.size + 1] = child.ptr[child.size];
            newbro.key_value[newbro.size] = cursor.key_value[l - 1];
            newbro.size += child.size + 1;
            for (int i = l - 1; i < cursor.size - 1; ++i) {
                cursor.key_value[i] = cursor.key_value[i + 1];
                cursor.ptr[i + 1] = cursor.ptr[i + 2];
            }
            --cursor.size;
            MR.update(cursor, pos);
            MR.update(newbro, cursor.ptr[l - 1]);
            if (cursor.size < MIN)
                return true;
            return false;
        } else if (l < cursor.size) {
            static node newbro;
            MR.read(newbro, cursor.ptr[l + 1]);
            if (newbro.size > MIN) {
                if (child.Is_leaf) {
                    MR.read(newbro, cursor.ptr[l + 1]);
                    child.key_value[child.size] = newbro.key_value[0];
                    ++child.size;
                    for (int i = 0; i < newbro.size - 1; ++i) {
                        newbro.key_value[i] = newbro.key_value[i + 1];
                    }
                    --newbro.size;
                    cursor.key_value[l] = newbro.key_value[0];
                    MR.update(cursor, pos);
                    MR.update(child, cursor.ptr[l]);
                    MR.update(newbro, cursor.ptr[l + 1]);
                    return false;
                }
                MR.read(newbro, cursor.ptr[l + 1]);
                child.key_value[child.size] = cursor.key_value[l];
                child.ptr[child.size + 1] = newbro.ptr[0];
                ++child.size;
                cursor.key_value[l] = newbro.key_value[0];
                for (int i = 0; i < newbro.size - 1; ++i) {
                    newbro.key_value[i] = newbro.key_value[i + 1];
                    newbro.ptr[i] = newbro.ptr[i + 1];
                }
                newbro.ptr[newbro.size - 1] = newbro.ptr[newbro.size];
                --newbro.size;
                MR.update(cursor, pos);
                MR.update(child, cursor.ptr[l]);
                MR.update(newbro, cursor.ptr[l + 1]);
                return false;
            }
            // 和右边合并
            if (child.Is_leaf) {
                MR.read(newbro, cursor.ptr[l + 1]);
                for (int i = 0; i < newbro.size; ++i) {
                    child.key_value[i + child.size] = newbro.key_value[i];
                }
                child.size += newbro.size;
                child.brother = newbro.brother;
                for (int i = l; i < cursor.size - 1; ++i) {
                    cursor.key_value[i] = cursor.key_value[i + 1];
                    cursor.ptr[i + 1] = cursor.ptr[i + 2];
                }
                --cursor.size;
                child.brother = newbro.brother;
                MR.update(cursor, pos);
                MR.update(child, cursor.ptr[l]);
                if (cursor.size < MIN)
                    return true;
                return false;
            }
            MR.read(newbro, cursor.ptr[l + 1]);
            for (int i = 0; i < newbro.size; ++i) {
                child.key_value[i + child.size + 1] = newbro.key_value[i];
                child.ptr[i + child.size + 1] = newbro.ptr[i];
            }
            child.ptr[child.size + newbro.size + 1] = newbro.ptr[newbro.size];
            child.key_value[child.size] = cursor.key_value[l];
            child.size += newbro.size + 1;
            for (int i = l; i < cursor.size - 1; ++i) {
                cursor.key_value[i] = cursor.key_value[i + 1];
                cursor.ptr[i + 1] = cursor.ptr[i + 2];
            }
            --cursor.size;
            MR.update(cursor,pos );
            MR.update(child, cursor.ptr[l]);
            if (cursor.size < MIN)
                return true;
            return false;
        }
        else {
            throw;
        }
    }

public:
    BPTree() {
        MR.initialise("BPTree.dat");
        MR.get_info(root, 1);
    }

    ~BPTree() {
        MR.write_info(root, 1);
        MR.end();
    };
    bool empty(){
        return root==-1;
    }

    void insert(char index[65], T value) {  //将kv插入B+树
        Key_Value<T> kv(index, value);
        if (root == -1) {//空树
            node x;
            x.key_value[0] = kv;
            x.size = 1;
            x.Is_leaf = true;
            x.brother = -1;
            root = MR.write(x);
        } else {
            node cursor;
            MR.read(cursor, root);
            insertInternal(cursor, root, kv);
        }
    }

    void delete_(char index[65], T value) {
        if (root == -1) return;
        Key_Value<T> kv(index, value);
        node cursor;
        MR.read(cursor, root);
        deleteInternal(cursor, root, kv);
    }

    std::vector<T> query(char index[65]) {
        std::vector<T> ans;
        ans.clear();
        if(root==-1){
            return ans;
        }
        node cursor;
        MR.read(cursor, root);
        while (!cursor.Is_leaf) {
            int i=0;
            for (; i < cursor.size; i++) {
                if (std::strcmp(index, cursor.key_value[i].index) <= 0
                    && (i-1==-1||std::strcmp(index, cursor.key_value[i-1].index) >= 0)) {
                    break;
                }
            }
            MR.read(cursor, cursor.ptr[i]);
        }
        for (int i = 0; i <= cursor.size; i++) {
            if (i == cursor.size) {
                if (cursor.brother == -1) {
                    break;
                }
                MR.read(cursor, cursor.brother);
                i = -1;
                continue;
            }
            if (std::strcmp(index, cursor.key_value[i].index) < 0) {
                break;
            }
            if (std::strcmp(index, cursor.key_value[i].index) == 0) {
                ans.push_back(cursor.key_value[i].value);
                continue;
            }
        }
        return ans;
    }

    /*void display(int cur) { //输出
        node cursor;
        MR.read(cursor, cur);
        std::cout << "size " << cursor.size << "||| ";
        for (int i = 0; i < cursor.size; i++) {
            std::cout << cursor.key_value[i].index << " " << cursor.key_value[i].value << ";";//size
        }
        std::cout << "\n";
        if (!cursor.Is_leaf) {
            for (int i = 0; i <= cursor.size; i++) {
                display(cursor.ptr[i]);
            }
        }
    }*/
    void end(){
        MR.end();
    }
};

#endif