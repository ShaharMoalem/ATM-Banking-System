#include "account.hpp"
#include "bank.hpp"
#include <ctime>
#include <iostream>
#include <vector>
#include <string>  

//#include <cstdio> 
//#include <chrono>
//#include <pthread.h>


Bank bank;
bool done = false;



void* ATM(void* arg);
void* bank_commission(void* arg);
void* print_bank(void* arg);

void milli_sleep(long milliseconds);
void second_sleep(long seconds);

pthread_mutex_t log_mutex;
std::ofstream output_file_log("log.txt", std::ofstream::out);

class ATMinput{
public: 
    int atm_id;
    std::vector<std::vector<std::string>> curr_file;
};

std::vector<std::vector<std::string>>* is_valid(int argc, char* argv[]);

void failure_operations(const std::string& error_message, pthread_t* ATMthreads, 
			ATMinput* ATMinputs, std::vector<std::vector<std::string>>* file_command_vectors);



int main(int argc, char* argv[]) {

    int num_of_files = argc - 1;
    std::vector<std::vector<std::string>>* file_command_vectors = is_valid(argc, argv);


    if (!file_command_vectors) 
    {
        failure_operations("Bank error: illegal arguments", nullptr, nullptr, file_command_vectors);
        exit(1);
    }

    if (pthread_mutex_init(&log_mutex, nullptr))
    {
        failure_operations("Bank error: pthread_mutex_init failed", nullptr, nullptr, file_command_vectors);
        exit(1);
    }


    // create Bank print thread
    int bank_id = 0;
    pthread_t bank_print_t;
    if (pthread_create(&bank_print_t, NULL, print_bank, (void*)&bank_id)) {
        failure_operations("Bank error: pthread_create failed", NULL, NULL, file_command_vectors);
        exit(1);
    }

   
    // create Bank commission thread
    int bank_commission_id = 0;
    pthread_t bank_commission_t;
    if (pthread_create(&bank_commission_t, NULL, bank_commission, (void*)&bank_commission_id)) {
        failure_operations("Bank error: pthread_create failed", NULL, NULL, file_command_vectors);
        exit(1);
    }

    // create ATMs threads 
    pthread_t* ATMthreads = new pthread_t[num_of_files];
    ATMinput* ATMinputs = new ATMinput[num_of_files];

    //each thread contains an ATM  
    for (int i = 0; i < num_of_files; i++) {
        ATMinputs[i].curr_file = file_command_vectors[i];
        ATMinputs[i].atm_id = i + 1;

        if (pthread_create(&ATMthreads[i], NULL, ATM, (void*)&ATMinputs[i])) {
            failure_operations("Bank error: pthread_create failed", ATMthreads, ATMinputs, file_command_vectors);
            exit(1);
        }
    }


    // ATMs threads join
    for (int i = 0; i < num_of_files; i++) {
        if (pthread_join(ATMthreads[i], NULL)) {
            failure_operations("Bank error: pthread_join failed", ATMthreads, ATMinputs, file_command_vectors);
            exit(1);
        }
    }
   //all atms done with all their commands
    done = true;
    if (pthread_join(bank_commission_t, NULL) || pthread_join(bank_print_t, NULL)) {
        failure_operations("Bank error: pthread_join failed", ATMthreads, ATMinputs, file_command_vectors);
        exit(1);
    }

    delete[] file_command_vectors;
    delete[] ATMthreads;
    delete[] ATMinputs;
    pthread_mutex_destroy_secure(&log_mutex);
    output_file_log.close();
    return 0;
}



void* ATM(void* arg) {
    ATMinput ATM_files = *(ATMinput*)arg;
    while (!ATM_files.curr_file.empty()) {

        // Initial sleep for 100 milliseconds
	milli_sleep(100);


        std::vector<std::string> curr_cmd = ATM_files.curr_file.front();//allocate current command linefrom current file in curr_cmd
        ATM_files.curr_file.erase(ATM_files.curr_file.begin());

        int atm_id = ATM_files.atm_id;
        int account_id = std::stoi(curr_cmd[1]);
        std::string password(curr_cmd[2]);
        int amount = 0;

       
        // Open_account
        if (curr_cmd[0] == "O")
        { 
	        int initial_amount = std::stoi(curr_cmd[3]);
            bank.open_account(account_id, password, initial_amount, atm_id);
        }
        // Close_account
        else if (curr_cmd[0] == "Q")
        {
            bank.close_account(atm_id, account_id, password);
        }
        // Deposit
        else if (curr_cmd[0] == "D")
        { 
			amount = std::stoi(curr_cmd[3]);
            bank.deposit_account(atm_id, account_id, password, amount);
        }
        // Withdraw
        else if (curr_cmd[0] == "W")
        {
			amount = std::stoi(curr_cmd[3]);
            bank.withdraw_account(atm_id, account_id, password, amount);

        }
        // Balance
        else if (curr_cmd[0] == "B")
        { 
            bank.get_balance(atm_id, account_id, password);
        }
        
        // Transfer
        else if (curr_cmd[0] == "T")
        { 
            amount = std::stoi(curr_cmd[4]);
            int target_id = std::stoi(curr_cmd[3]);
            bank.transfer(atm_id, account_id, password,  target_id,amount);
        }
        // Post-transaction sleep for 1 second
        second_sleep(1);
    }

    pthread_exit(NULL);
}


void* bank_commission(void* arg) {
    while (!done) {
        bank.commission();
        second_sleep(3);
    }
    pthread_exit(NULL);
}

void* print_bank(void* arg) {
    while (!done) {
        bank.print_bank();
	milli_sleep(500);
    }
    pthread_exit(NULL);
}

void milli_sleep(long milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;               // Convert milliseconds to seconds
    ts.tv_nsec = (milliseconds % 1000) * 1000000;  // Convert the remainder to nanoseconds
    while (nanosleep(&ts, &ts) && errno == EINTR); // Retry if interrupted by a signal
}

void second_sleep(long seconds) {
    struct timespec ts;
    ts.tv_sec = seconds;
    ts.tv_nsec = 0;
    while (nanosleep(&ts, &ts) && errno == EINTR);
}

void failure_operations(const std::string& error_message, pthread_t* ATMthreads, ATMinput* ATMinputs, std::vector<std::vector<std::string>>* file_command_vectors) {
    std::perror(error_message.c_str());
    if (ATMinputs) {
        delete[] ATMinputs;
    }
    if (ATMthreads) {
        delete[] ATMthreads;
    }
    if (file_command_vectors) {
        delete[] file_command_vectors;
    }
    output_file_log.close();
}

std::vector<std::vector<std::string>>* is_valid(int argc, char* argv[]) {
    if (argc < MIN_NUM_ARGC)
        return nullptr;

    std::vector<std::vector<std::string>>* file_command_vectors = new std::vector<std::vector<std::string>>[argc - 1];

    //iterates over the files input and allocates each command in a file vector 
    for (int i = 1; i < argc; i++) {
        std::ifstream curr_File(argv[i]);
        if (!curr_File.is_open())
        {
            delete[] file_command_vectors;
            return NULL;
        }
        std::string curr_line;
        while (std::getline(curr_File, curr_line)) {
            file_command_vectors[i - 1].push_back(split_input(curr_line));
        }
        curr_File.close();
    }
    return file_command_vectors;
}
