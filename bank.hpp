#ifndef BANK_HPP
#define BANK_HPP

#include "account.hpp"
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <random>

#include <string>

#define INITIAL_PROFIT 0
#define MIN_NUM_ARGC 2

extern pthread_mutex_t log_mutex;
extern std::ofstream output_file_log;

std::vector<std::string> split_input(std::string const& input, const char delimiters = ' ');

class Bank { 
private:
    int profit;
    std::vector<Account> acc_list;
    int readers_num;//for semaphore
    pthread_mutex_t bread_mutex;//bank reade mutex
    pthread_mutex_t bwrite_mutex;//bank write mutex

 // Helper functions
    int random_one_to_five();
    std::vector<std::string> split_input(std::string const& input, const char delimiters);
    void sort_acc_list();
    bool account_exists(int account_id) const;
    Account* find_account(int account_id);
    
    
public:
    Bank();//constructor
    ~Bank();//destructor

    void read_lock();
    void read_unlock();
    void write_lock();
    void write_unlock();
    void print_list();
    void open_account(int account_id, std::string input_pin, int initial_amount,int atm_id);
    void close_account(int atm_id, int account_id, std::string input_pin);
    int get_profit();
    void deposit_account(int atm_id, int account_id, std::string input_pin, int amount);
    void withdraw_account(int atm_id, int account_id, std::string input_pin, int amount);
    void get_balance(int atm_id, int account_id, std::string input_pin);
    void transfer(int atm_id, int account_id, std::string input_pin, int target_id, int amount);
    void commission();
    void print_bank();
};

#endif
