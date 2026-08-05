#include <bits/stdc++.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <future>
#include <atomic>
#include <fstream>

struct termios old_terminal_settings;

void init_terminal() {
    struct termios new_settings;
    if (tcgetattr(0, &old_terminal_settings) < 0) {
        perror("tcgetattr");
    }
    new_settings = old_terminal_settings;
    
    new_settings.c_lflag &= ~(ICANON | ECHO);
    new_settings.c_cc[VMIN] = 0;   
    new_settings.c_cc[VTIME] = 0;  
        
    if (tcsetattr(0, TCSANOW, &new_settings) < 0) {
        perror("tcsetattr raw mode");
    }
}

void restore_terminal() {
    if (tcsetattr(0, TCSADRAIN, &old_terminal_settings) < 0) {
        perror("tcsetattr restore");
    }
}

char getch() {
    char buf = 0;
    if (read(0, &buf, 1) < 0) {
    }
    return buf;
}

void timer(std::atomic<int>& timeval, std::atomic<bool>& over) {
    while (timeval > 0 && !over) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timeval--;
    }
    over = true;
}

int load_highscore() {
    int hs = 0;
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> hs;
        file.close();
    }
    return hs;
}

void save_highscore(int hs) {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << hs;
        file.close();
    }
}

int main(){ 
    init_terminal();
    std::cout << "\033[H\033[2J";

    int high_score = load_highscore();

    std::string cur = "xxxx";

    std::random_device dev;
    std::mt19937 mt(dev());
    std::uniform_int_distribution<int> dist(0,3);

    int start = dist(mt);
    int pos = 0;

    cur[pos] = 'o';
    cur[start] = '*';

    int score = 0;
    
    std::atomic<int> timeval(15); 
    std::atomic<bool> over(false);
    
    std::future<void> fut = std::async(std::launch::async, timer, std::ref(timeval), std::ref(over));

    while(!over){
        for(int i = 0; i < 4; i++) {
            cur[i] = 'x';
        }

        cur[start] = '*';
        
        if(pos == start){
            cur[pos] = '#';
        } else {
            cur[pos] = 'o';
        }

        std::cout << "\033[H\033[2J";
        std::cout << "Time Left: " << timeval << "s" << std::endl;
        std::cout << "High Score: " << high_score << std::endl;
        std::cout << "Score: " << score << std::endl;
        std::cout << cur << std::endl;
        std::cout.flush(); 
    
        char ch = getch();

        if(ch == 32){ 
            if(pos == start){
                score++;
                start = dist(mt);
                continue;
            } else {
                if (score > 0) {
                    score--;
                }
                timeval = timeval - 2;
                if (timeval <= 0) {
                    over = true;
                }
            }
        }

        if(ch == 27){ 
            auto start_time = std::chrono::steady_clock::now();
            char next = 0;
            while (next == 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() < 15) {
                next = getch();
            }
            
            char another = 0;
            if (next != 0) {
                start_time = std::chrono::steady_clock::now();
                while (another == 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count() < 15) {
                    another = getch();
                }
            }

            if(next == '['){
                if(another == 'D'){ 
                    if(pos != 0) pos--;
                    else pos = 3;
                }
                if(another == 'C'){ 
                    if(pos != 3) pos++;
                    else pos = 0;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); 
    }
    
    bool new_record = false;
    if (score > high_score) {
        high_score = score;
        save_highscore(high_score);
        new_record = true;
    }

    std::cout << "\033[H\033[2J";
    std::cout << "==========================" << std::endl;
    std::cout << "        GAME OVER!        " << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "Final Score: " << score << std::endl;
    if (new_record) {
        std::cout << "NEW HIGH SCORE! Saved." << std::endl;
    } else {
        std::cout << "High Score to Beat: " << high_score << std::endl;
    }

    restore_terminal();
    return 0;
}
