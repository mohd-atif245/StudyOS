#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <termios.h>
#include <sys/select.h>

using namespace std;

// ========== SIMPLE COLORS ==========
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

struct Task {
    int id;
    string name;
    string status;
    string created;
};

struct Note {
    int id;
    string content;
    string created;
};

vector<Task> tasks;
vector<Note> notes;
int totalSessions = 0;
int totalFocusTime = 0;

void clearScreen() { system("clear"); }

string getCurrentDateTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string datetime(dt);
    return datetime.substr(0, datetime.length() - 1);
}

string getCurrentDate() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string date(dt);
    return date.substr(0, 10);
}

string getCurrentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string timeStr(dt);
    return timeStr.substr(11, 8);
}

void printHeader() {
    cout << BOLD << CYAN << "\n================================================" << RESET << endl;
    cout << BOLD << CYAN << "  STUDY OS" << RESET << endl;
    cout << CYAN << "  Your Productivity Companion" << RESET << endl;
    cout << BOLD << CYAN << "================================================" << RESET << endl;
}

void printFooter() {
    cout << "\n" << CYAN << "────────────────────────────────────────────────" << RESET << endl;
    cout << "📅 " << getCurrentDate() << "          🕐 " << getCurrentTime() << endl;
    cout << CYAN << "────────────────────────────────────────────────" << RESET << endl;
}

void loadData() {
    tasks.clear();
    notes.clear();
    
    ifstream taskFile("tasks.txt");
    if (taskFile.is_open()) {
        Task t;
        string line;
        while (getline(taskFile, line)) {
            size_t p1 = line.find('|');
            size_t p2 = line.find('|', p1 + 1);
            size_t p3 = line.find('|', p2 + 1);
            if (p1 != string::npos && p2 != string::npos) {
                try {
                    t.id = stoi(line.substr(0, p1));
                    t.name = line.substr(p1 + 1, p2 - p1 - 1);
                    t.status = line.substr(p2 + 1, p3 - p2 - 1);
                    t.created = (p3 + 1 < line.length()) ? line.substr(p3 + 1) : getCurrentDateTime();
                    if (t.name.length() > 0 && t.name.length() < 200) {
                        tasks.push_back(t);
                    }
                } catch(...) {}
            }
        }
        taskFile.close();
    }
    
    ifstream noteFile("notes.txt");
    if (noteFile.is_open()) {
        Note n;
        string line;
        while (getline(noteFile, line)) {
            size_t p1 = line.find('|');
            size_t p2 = line.find('|', p1 + 1);
            if (p1 != string::npos && p2 != string::npos) {
                try {
                    n.id = stoi(line.substr(0, p1));
                    n.content = line.substr(p1 + 1, p2 - p1 - 1);
                    n.created = (p2 + 1 < line.length()) ? line.substr(p2 + 1) : getCurrentDateTime();
                    if (n.content.length() > 0 && n.content.length() < 500) {
                        notes.push_back(n);
                    }
                } catch(...) {}
            }
        }
        noteFile.close();
    }
    
    ifstream statsFile("stats.txt");
    if (statsFile.is_open()) {
        statsFile >> totalSessions >> totalFocusTime;
        statsFile.close();
    }
}

void saveTasks() {
    ofstream taskFile("tasks.txt");
    for (const auto& t : tasks) {
        taskFile << t.id << "|" << t.name << "|" << t.status << "|" << t.created << endl;
    }
    taskFile.close();
}

void saveNotes() {
    ofstream noteFile("notes.txt");
    for (const auto& n : notes) {
        noteFile << n.id << "|" << n.content << "|" << n.created << endl;
    }
    noteFile.close();
}

void saveStats() {
    ofstream statsFile("stats.txt");
    statsFile << totalSessions << " " << totalFocusTime << endl;
    statsFile.close();
}

void saveAllData() {
    saveTasks();
    saveNotes();
    saveStats();
}

bool kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

void tasksMenu() {
    while (true) {
        clearScreen();
        printHeader();
        
        if (tasks.empty()) {
            cout << "\n  " << YELLOW << "📭 No tasks yet. Add your first task!" << RESET << endl;
        } else {
            cout << "\n  " << BOLD << CYAN << "YOUR TASKS" << RESET << endl;
            cout << "  " << CYAN << "----------------------------------------" << RESET << endl;
            printf("  %sID   Task                          Status%s\n", CYAN, RESET);
            cout << "  " << CYAN << "----------------------------------------" << RESET << endl;
            
            for (const auto& t : tasks) {
                string statusColor = (t.status == "Completed") ? GREEN : YELLOW;
                string statusIcon = (t.status == "Completed") ? "✓" : "○";
                string taskName = t.name;
                if (taskName.length() > 26) {
                    taskName = taskName.substr(0, 23) + "...";
                }
                printf("  %2d   %-26s %s%c %s%s\n", 
                       t.id, taskName.c_str(), statusColor.c_str(), statusIcon.c_str(), t.status.c_str(), RESET);
            }
            cout << "  " << CYAN << "----------------------------------------" << RESET << endl;
        }
        
        cout << "\n  " << BOLD << YELLOW << "TASKS MENU" << RESET << endl;
        cout << "  " << GREEN << "1. Add Task" << RESET << endl;
        cout << "  " << GREEN << "2. Complete Task" << RESET << endl;
        cout << "  " << RED << "3. Delete Task" << RESET << endl;
        cout << "  " << CYAN << "4. Back" << RESET << endl;
        
        int choice;
        cout << "\n  " << CYAN << "Select: " << RESET;
        cin >> choice;
        cin.ignore();
        
        if (choice == 1) {
            string name;
            cout << "  " << CYAN << "Enter task: " << RESET;
            getline(cin, name);
            if (name.length() > 0 && name.length() < 100) {
                Task newTask;
                newTask.id = tasks.size() + 1;
                newTask.name = name;
                newTask.status = "Pending";
                newTask.created = getCurrentDateTime();
                tasks.push_back(newTask);
                saveTasks();
                cout << "\n  " << GREEN << "✓ Task added!" << RESET << endl;
            } else {
                cout << "\n  " << RED << "✗ Invalid task!" << RESET << endl;
            }
            sleep(1);
        }
        else if (choice == 2) {
            if (tasks.empty()) {
                cout << "\n  " << YELLOW << "No tasks to complete!" << RESET << endl;
                sleep(1);
                continue;
            }
            int id;
            cout << "  " << CYAN << "Enter task ID: " << RESET;
            cin >> id;
            bool found = false;
            for (auto& t : tasks) {
                if (t.id == id) {
                    found = true;
                    if (t.status == "Completed") {
                        cout << "\n  " << YELLOW << "Task already completed!" << RESET << endl;
                    } else {
                        t.status = "Completed";
                        saveTasks();
                        cout << "\n  " << GREEN << "✓ Task completed!" << RESET << endl;
                    }
                    break;
                }
            }
            if (!found) {
                cout << "\n  " << RED << "✗ Task not found!" << RESET << endl;
            }
            sleep(1);
        }
        else if (choice == 3) {
            if (tasks.empty()) {
                cout << "\n  " << YELLOW << "No tasks to delete!" << RESET << endl;
                sleep(1);
                continue;
            }
            int id;
            cout << "  " << CYAN << "Enter task ID: " << RESET;
            cin >> id;
            bool found = false;
            for (auto it = tasks.begin(); it != tasks.end(); ++it) {
                if (it->id == id) {
                    tasks.erase(it);
                    for (size_t i = 0; i < tasks.size(); i++) {
                        tasks[i].id = i + 1;
                    }
                    saveTasks();
                    cout << "\n  " << GREEN << "✓ Task deleted!" << RESET << endl;
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "\n  " << RED << "✗ Task not found!" << RESET << endl;
            }
            sleep(1);
        }
        else if (choice == 4) {
            break;
        }
    }
}

void timerMenu() {
    clearScreen();
    printHeader();
    
    cout << "\n  " << BOLD << YELLOW << "POMODORO TIMER" << RESET << endl;
    cout << "  " << CYAN << "Session: Focus Time (25 minutes)" << RESET << endl;
    cout << "\n  " << YELLOW << "Press 'q' to stop the timer" << RESET << endl;
    cout << "\n  " << CYAN << "Press Enter to start..." << RESET;
    cin.ignore();
    cin.get();
    
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    bool stopped = false;
    
    for (int i = 1500; i > 0 && !stopped; i--) {
        int minutes = i / 60;
        int seconds = i % 60;
        
        clearScreen();
        printHeader();
        
        cout << "\n  " << BOLD << GREEN << "FOCUS SESSION" << RESET << endl;
        printf("  Time: %02d:%02d\n", minutes, seconds);
        cout << "\n  " << YELLOW << "Press 'q' to stop" << RESET << endl;
        
        if (kbhit()) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) == 1) {
                if (ch == 'q' || ch == 'Q') {
                    stopped = true;
                    cout << "\n  " << RED << "Timer stopped." << RESET << endl;
                }
            }
        }
        
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    if (!stopped) {
        cout << "\n  " << BOLD << GREEN << "🎉 TIME'S UP! Great job! 🎉" << RESET << endl;
        totalSessions++;
        totalFocusTime += 25;
        saveStats();
    }
    
    cout << "\n  " << CYAN << "Press Enter to continue..." << RESET;
    cin.get();
}

void notesMenu() {
    while (true) {
        clearScreen();
        printHeader();
        
        if (notes.empty()) {
            cout << "\n  " << YELLOW << "📭 No notes yet. Add your first note!" << RESET << endl;
        } else {
            cout << "\n  " << BOLD << CYAN << "YOUR NOTES" << RESET << endl;
            cout << "  " << CYAN << "----------------------------------------" << RESET << endl;
            printf("  %sID   Note%s\n", CYAN, RESET);
            cout << "  " << CYAN << "----------------------------------------" << RESET << endl;
            
            for (const auto& n : notes) {
                string content = n.content;
                if (content.length() > 36) {
                    content = content.substr(0, 33) + "...";
                }
                printf("  %2d   %s\n", n.id, content.c_str());
            }
            cout << "  " << CYAN << "----------------------------------------" << RESET << endl;
        }
        
        cout << "\n  " << BOLD << YELLOW << "NOTES MENU" << RESET << endl;
        cout << "  " << GREEN << "1. Add Note" << RESET << endl;
        cout << "  " << RED << "2. Delete Note" << RESET << endl;
        cout << "  " << CYAN << "3. Back" << RESET << endl;
        
        int choice;
        cout << "\n  " << CYAN << "Select: " << RESET;
        cin >> choice;
        cin.ignore();
        
        if (choice == 1) {
            string content;
            cout << "  " << CYAN << "Enter note: " << RESET;
            getline(cin, content);
            if (content.length() > 0 && content.length() < 200) {
                Note newNote;
                newNote.id = notes.size() + 1;
                newNote.content = content;
                newNote.created = getCurrentDateTime();
                notes.push_back(newNote);
                saveNotes();
                cout << "\n  " << GREEN << "✓ Note added!" << RESET << endl;
            } else {
                cout << "\n  " << RED << "✗ Invalid note!" << RESET << endl;
            }
            sleep(1);
        }
        else if (choice == 2) {
            if (notes.empty()) {
                cout << "\n  " << YELLOW << "No notes to delete!" << RESET << endl;
                sleep(1);
                continue;
            }
            int id;
            cout << "  " << CYAN << "Enter note ID: " << RESET;
            cin >> id;
            bool found = false;
            for (auto it = notes.begin(); it != notes.end(); ++it) {
                if (it->id == id) {
                    notes.erase(it);
                    for (size_t i = 0; i < notes.size(); i++) {
                        notes[i].id = i + 1;
                    }
                    saveNotes();
                    cout << "\n  " << GREEN << "✓ Note deleted!" << RESET << endl;
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "\n  " << RED << "✗ Note not found!" << RESET << endl;
            }
            sleep(1);
        }
        else if (choice == 3) {
            break;
        }
    }
}

void showDashboard() {
    clearScreen();
    printHeader();
    
    int completed = 0;
    for (const auto& t : tasks) {
        if (t.status == "Completed") completed++;
    }
    int pending = tasks.size() - completed;
    int rate = (tasks.size() > 0) ? (completed * 100 / tasks.size()) : 0;
    
    cout << "\n  " << BOLD << YELLOW << "DASHBOARD" << RESET << endl;
    cout << "  " << CYAN << "────────────────────────────────────────" << RESET << endl;
    cout << "  " << CYAN << "📋 Total Tasks:" << RESET << "     " << tasks.size() << endl;
    cout << "  " << GREEN << "✅ Completed:" << RESET << "       " << completed << endl;
    cout << "  " << YELLOW << "⏳ Pending:" << RESET << "         " << pending << endl;
    cout << "  " << MAGENTA << "📊 Completion Rate:" << RESET << "  " << rate << "%" << endl;
    cout << "  " << CYAN << "🎯 Study Sessions:" << RESET << "  " << totalSessions << endl;
    cout << "  " << CYAN << "⏱️  Focus Time:" << RESET << "     " << totalFocusTime << " mins" << endl;
    cout << "  " << CYAN << "📝 Total Notes:" << RESET << "     " << notes.size() << endl;
    cout << "  " << CYAN << "────────────────────────────────────────" << RESET << endl;
    
    cout << "\n  " << BOLD << YELLOW << "STUDY ANALYTICS" << RESET << endl;
    cout << "  " << CYAN << "This Week" << RESET << endl;
    cout << "  " << CYAN << "────────────────────────────────────────" << RESET << endl;
    cout << "  " << CYAN << "Study Sessions:" << RESET << "  " << totalSessions << endl;
    cout << "  " << CYAN << "Total Focus Time:" << RESET << " " << totalFocusTime << " mins" << endl;
    cout << "  " << CYAN << "Tasks Completed:" << RESET << "  " << completed << endl;
    
    cout << "\n  " << BOLD << GREEN;
    if (rate == 100 && tasks.size() > 0) {
        cout << "🏆 Perfect! All tasks completed!" << RESET << endl;
    } else if (rate >= 75) {
        cout << "🌟 Excellent progress! Keep going!" << RESET << endl;
    } else if (rate >= 50) {
        cout << "💪 Good job! More than halfway there!" << RESET << endl;
    } else if (rate >= 25) {
        cout << "📈 Making progress! Stay consistent!" << RESET << endl;
    } else if (tasks.size() > 0) {
        cout << "🚀 Complete your first task today!" << RESET << endl;
    } else {
        cout << "✨ Add tasks and begin your journey!" << RESET << endl;
    }
    
    cout << "\n  " << CYAN << "Press Enter to continue..." << RESET;
    cin.ignore();
    cin.get();
}

int main() {
    loadData();
    
    while (true) {
        clearScreen();
        printHeader();
        
        cout << "\n  " << BOLD << YELLOW << "MAIN MENU" << RESET << endl;
        cout << "  " << CYAN << "────────────────────────────────────────" << RESET << endl;
        cout << "  " << GREEN << "1. Tasks" << RESET << endl;
        cout << "  " << GREEN << "2. Pomodoro Timer" << RESET << endl;
        cout << "  " << GREEN << "3. Notes" << RESET << endl;
        cout << "  " << GREEN << "4. Dashboard" << RESET << endl;
        cout << "  " << RED << "5. Exit" << RESET << endl;
        printFooter();
        
        int choice;
        cout << "\n  " << CYAN << "Select option: " << RESET;
        cin >> choice;
        
        switch(choice) {
            case 1: tasksMenu(); break;
            case 2: timerMenu(); break;
            case 3: notesMenu(); break;
            case 4: showDashboard(); break;
            case 5:
                cout << "\n  " << YELLOW << "Saving data..." << RESET << endl;
                saveAllData();
                cout << "  " << GREEN << "Goodbye! Keep studying! 📚" << RESET << endl;
                return 0;
            default:
                cout << "\n  " << RED << "Invalid option!" << RESET << endl;
                sleep(1);
        }
    }
    return 0;
}