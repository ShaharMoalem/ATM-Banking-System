#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP
#include <string>
#include <pthread.h> 
#include <iostream>
#include <fstream>
#include <cstdint>
#include <unistd.h>
#include <cmath>


#define INITIAL_NUMBER_OF_READERS 0



extern pthread_mutex_t log_mutex;
extern std::ofstream output_file_log;

//instead of the normal pthreads funcs, to print errors and close the files
void pthread_mutex_init_secure(pthread_mutex_t* mutex);
void pthread_mutex_lock_secure(pthread_mutex_t* mutex);
void pthread_mutex_unlock_secure(pthread_mutex_t* mutex);
void pthread_mutex_destroy_secure(pthread_mutex_t* mutex);

class Account { 
private:
    int account_id;//a public field to access it by the bank -for the vector of accounts 
    int balance;
    std::string acc_pin;// account pin (password)
    int readers_num;
    pthread_mutex_t aread_mutex;//acount read mutex
    pthread_mutex_t awrite_mutex;//account write

public:
    //int account_id;//a public field to access it by the bank -for the vector of accounts
    int getAccountId() const {return account_id; }
    Account();//defualt constructor
    Account(int input_id, int input_balance, std::string input_pin);//constructor for making a new account
    ~Account();//need a destructor for the mutex 
    bool pin_compare(std::string input_pin) const;
    void readLock();
    void readUnlock();
    void writeLock();
    void writeUnlock();
    void get_balance(int atm_id, int account_id,const std::string input_pin);
    std::string get_pin() const;
    void deposit(int atm_id, int account_id, std::string input_pin, int amount);
    void withdraw(int atm_id, int account_id, std::string input_pin, int amount);
    int commission_acc(int percent);//MAKE SURE TO LOCK BEFORE AND AFTER USE IN BANK
    int get_balance_wo_lock() const;
    int get_balance_bi();
    void update_balance(int new_balance);
};


#endif
