
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
using namespace std;

// DSA 1: Stack for Back Navigation
struct StackNode {
    string path;
    StackNode* next;
};

class PathStack {
    StackNode* topNode;

public:
    PathStack() : topNode(NULL) {}

    void push(string p) {
        StackNode* n = new StackNode;
        n->path = p;
        n->next = topNode;
        topNode = n;
    }

    bool isEmpty() {
        return topNode == NULL;
    }

    string pop() {
        if (isEmpty()) return "";
        StackNode* temp = topNode;
        string p = temp->path;
        topNode = topNode->next;
        delete temp;
        return p;
    }
};

// DSA 2: Linked List for Recent Files
struct FileNode {
    string name;
    FileNode* next;
};

class RecentFiles {
    FileNode* head;
    int count;

public:
    RecentFiles() : head(NULL), count(0) {}

    void add(string name) {
        FileNode* n = new FileNode;
        n->name = name;
        n->next = head;
        head = n;
        count++;

        if (count > 10) {
            FileNode* cur = head;
            while (cur->next && cur->next->next)
                cur = cur->next;
            if (cur->next) {
                delete cur->next;
                cur->next = NULL;
            }
            count = 10;
        }
    }

    void display() {
        cout << "\n--- Recent Files ---\n";
        if (!head) cout << "No recent files yet.\n";
        FileNode* cur = head;
        while (cur) {
            cout << "- " << cur->name << "\n";
            cur = cur->next;
        }
    }
};

PathStack history;
RecentFiles recent;

string currentPath() {
    char buffer[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buffer))
        return string(buffer);
    return ".";
}

bool existsItem(string name) {
    DWORD attr = GetFileAttributesA(name.c_str());
    return attr != INVALID_FILE_ATTRIBUTES;
}

bool isFolder(string name) {
    DWORD attr = GetFileAttributesA(name.c_str());
    return attr != INVALID_FILE_ATTRIBUTES &&
           (attr & FILE_ATTRIBUTE_DIRECTORY);
}

void listDirectory() {
    cout << "\n--- Files and Folders ---\n";
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA("*", &data);

    if (h == INVALID_HANDLE_VALUE) {
        cout << "Cannot list this folder.\n";
        return;
    }

    do {
        string name = data.cFileName;
        if (name != "." && name != "..") {
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                cout << "[FOLDER] " << name << "\n";
            else
                cout << "[FILE]   " << name << "\n";
        }
    } while (FindNextFileA(h, &data));

    FindClose(h);
}

void createFolder() {
    string name;
    cout << "Folder name: ";
    cin >> name;

    if (_mkdir(name.c_str()) == 0)
        cout << "Folder created successfully!\n";
    else
        cout << "Could not create folder. It may already exist.\n";
}

void openFolder() {
    string name;
    cout << "Folder name to open: ";
    cin >> name;

    if (isFolder(name)) {
        history.push(currentPath());
        if (SetCurrentDirectoryA(name.c_str()))
            cout << "Opened: " << currentPath() << "\n";
        else
            cout << "Could not open folder.\n";
    } else {
        cout << "Folder not found!\n";
    }
}

void goBack() {
    if (history.isEmpty()) {
        cout << "No previous folder.\n";
        return;
    }

    string p = history.pop();
    if (SetCurrentDirectoryA(p.c_str()))
        cout << "Back to: " << currentPath() << "\n";
    else
        cout << "Could not return to previous folder.\n";
}

void createFile() {
    string name, content;
    cout << "File name: ";
    cin >> name;

    ofstream file(name.c_str());
    if (!file) {
        cout << "Could not create file.\n";
        return;
    }

    cin.ignore(10000, '\n');
    cout << "Enter content: ";
    getline(cin, content);
    file << content;
    file.close();

    recent.add(name);
    cout << "File created successfully!\n";
}

void readFile() {
    string name, line;
    cout << "File name to read: ";
    cin >> name;

    ifstream file(name.c_str());
    if (!file) {
        cout << "File not found!\n";
        return;
    }

    cout << "\n--- File Content ---\n";
    while (getline(file, line))
        cout << line << "\n";

    recent.add(name);
}

void appendFile() {
    string name, content;
    cout << "File name: ";
    cin >> name;

    ofstream file(name.c_str(), ios::app);
    if (!file) {
        cout << "Could not open file.\n";
        return;
    }

    cin.ignore(10000, '\n');
    cout << "Text to append: ";
    getline(cin, content);
    file << content << "\n";
    cout << "Text added successfully!\n";
    recent.add(name);
}

void renameItem() {
    string oldName, newName;
    cout << "Current name: ";
    cin >> oldName;
    cout << "New name: ";
    cin >> newName;

    if (!existsItem(oldName)) {
        cout << "Item not found!\n";
    } else if (rename(oldName.c_str(), newName.c_str()) == 0) {
        cout << "Renamed successfully!\n";
    } else {
        cout << "Rename failed.\n";
    }
}

void deleteItem() {
    string name;
    cout << "Name to delete: ";
    cin >> name;

    if (isFolder(name)) {
        cout << "For safety, delete files only; folder deletion is disabled.\n";
        return;
    }

    if (remove(name.c_str()) == 0)
        cout << "File deleted!\n";
    else
        cout << "Could not delete. Check the file name.\n";
}

void searchItem() {
    string keyword;
    cout << "Enter file/folder name to search in current folder: ";
    cin >> keyword;

    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA("*", &data);
    bool found = false;

    if (h == INVALID_HANDLE_VALUE) {
        cout << "Search failed.\n";
        return;
    }

    do {
        string name = data.cFileName;
        if (name != "." && name != ".." &&
            name.find(keyword) != string::npos) {
            cout << "Found: " << name << "\n";
            found = true;
        }
    } while (FindNextFileA(h, &data));

    FindClose(h);
    if (!found) cout << "No matching items found.\n";
}

void fileInfo() {
    string name;
    cout << "File or folder name: ";
    cin >> name;

    if (!existsItem(name)) {
        cout << "Item not found!\n";
        return;
    }

    cout << "Name: " << name << "\n";
    if (isFolder(name)) {
        cout << "Type: Folder\n";
    } else {
        cout << "Type: File\n";
        struct _stat info;
        if (_stat(name.c_str(), &info) == 0)
            cout << "Size: " << info.st_size << " bytes\n";
    }
}

int main() {
    int choice;

    do {
        cout << "\n====================================\n";
        cout << "      FILE MANAGEMENT SYSTEM\n";
        cout << "====================================\n";
        cout << "Current Folder: " << currentPath() << "\n\n";
        cout << "1. List Files and Folders\n";
        cout << "2. Open Folder\n";
        cout << "3. Back\n";
        cout << "4. Create Folder\n";
        cout << "5. Create File\n";
        cout << "6. Read File\n";
        cout << "7. Append to File\n";
        cout << "8. Rename Item\n";
        cout << "9. Delete File\n";
        cout << "10. Search\n";
        cout << "11. File Information\n";
        cout << "12. Recent Files\n";
        cout << "13. Exit\n";
        cout << "Choose option: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1: listDirectory(); break;
            case 2: openFolder(); break;
            case 3: goBack(); break;
            case 4: createFolder(); break;
            case 5: createFile(); break;
            case 6: readFile(); break;
            case 7: appendFile(); break;
            case 8: renameItem(); break;
            case 9: deleteItem(); break;
            case 10: searchItem(); break;
            case 11: fileInfo(); break;
            case 12: recent.display(); break;
            case 13: cout << "Thank you for using File Manager!\n"; break;
            default: cout << "Invalid option!\n";
        }

    } while (choice != 13);

    return 0;
}
