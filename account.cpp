#include "account.hpp"
#include <typeinfo>

void pthread_mutex_init_secure(pthread_mutex_t* mutex) {
    if (pthread_mutex_init(mutex,NULL)) {
        std::perror("Bank error: pthread_mutex_init failed");
        output_file_log.close();
        exit(1);
    }
}


void pthread_mutex_lock_secure(pthread_mutex_t* mutex) {
    if (pthread_mutex_lock(mutex)) {
        std::perror("Bank error: pthread_mutex_lock failed");
        output_file_log.close();
        exit(1);
    }
}

void pthread_mutex_unlock_secure(pthread_mutex_t* mutex) {
    if (pthread_mutex_unlock(mutex)) {
        std::perror("Bank error: pthread_mutex_unlock failed");
        output_file_log.close();
        exit(1);
    }
}

void pthread_mutex_destroy_secure(pthread_mutex_t* mutex) {
    if (pthread_mutex_destroy(mutex)) {
        std::perror("Bank error: pthread_mutex_destroy failed");
        output_file_log.close();
        exit(1);
    }
}


/*//#############################Account functions#############################\\*/

//int Account::getAccountId(){return account_id;}

Account::Account() {
    pthread_mutex_init_secure(&aread_mutex);
    pthread_mutex_init_secure(&awrite_mutex);
}
Account::Account(int input_id, int input_balance, std::string input_pin) :account_id(input_id),
    balance(input_balance), acc_pin(input_pin), readers_num(INITIAL_NUMBER_OF_READERS) {
    pthread_mutex_init_secure(&aread_mutex);
    pthread_mutex_init_secure(&awrite_mutex);
}

Account::~Account() {
    pthread_mutex_destroy_secure(&aread_mutex);
    pthread_mutex_destroy_secure(&awrite_mutex);
}

std::string Account::get_pin() const {
    return this->acc_pin;
}
bool Account::pin_compare(std::string input_pin) const {
	return ( this->get_pin() == input_pin );
	/*if (this->get_pin() == input_pin) {
		 pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "the return is: " << (this->get_pin() == input_pin)<< std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
		return true;
	}
	else {
		return false;
	}*/
}
int wl = 0;
int wul = 0 ;
void Account::writeLock() {
    pthread_mutex_lock_secure(&awrite_mutex);
	/*pthread_mutex_lock_secure(&log_mutex);
		wl++;
        output_file_log << "write locks " << wl << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
}
void Account::writeUnlock() {
    pthread_mutex_unlock_secure(&awrite_mutex);/*
	pthread_mutex_lock_secure(&log_mutex);
		wul++;
        output_file_log << "write unlocks " << wl<<std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
}
 int rl =0 ;
 int rul = 0 ;
void Account::readLock() {
    pthread_mutex_lock_secure(&aread_mutex);
    this->readers_num += 1;
    if (this->readers_num == 1) {
        this->writeLock();
    }
	/*pthread_mutex_lock_secure(&log_mutex);
	rl++;
        output_file_log << "read locks "<< rl << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
    pthread_mutex_unlock_secure(&aread_mutex);
}
void Account::readUnlock() {
    pthread_mutex_lock_secure(&aread_mutex);
    this->readers_num -= 1;
    if (this->readers_num == 0) {
        this->writeUnlock();
    }
	/*pthread_mutex_lock_secure(&log_mutex);
	rul++;
        output_file_log << "read unlocks "<< rl << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
    pthread_mutex_unlock_secure(&aread_mutex);
}

void Account::get_balance(int atm_id, int account_id, std::string input_pin) {
    if (!this->pin_compare(input_pin)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - password for account id "
            << account_id << " is incorrect"<<std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
	else {
		this->readLock();
		pthread_mutex_lock_secure(&log_mutex);
		output_file_log << atm_id << ": Account " << account_id << " balance is " 
			<< this->balance << std::endl;
		pthread_mutex_unlock_secure(&log_mutex);
		this->readUnlock();
	}
}

void Account::deposit(int atm_id, int account_id, std::string input_pin, int amount) {

    if(this->pin_compare(input_pin)) {
		this->writeLock();
        int new_balance = this->get_balance_bi() + amount;
        this->update_balance(new_balance);
        this->writeUnlock();
        pthread_mutex_lock_secure(&log_mutex);
		output_file_log << atm_id << ": Account " << account_id << " new balance is " 
            << this->balance << " after " << amount << " $ was deposited" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);

    }
    else {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - password for account id " 
            << account_id << " is incorrect" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
        return;
    }
}

void Account::withdraw(int atm_id, int account_id, std::string input_pin, int amount) {
    if (this->pin_compare(input_pin)) {
        if (this->balance < amount) {
            pthread_mutex_lock_secure(&log_mutex);
            output_file_log << "Error " << atm_id << ": Your transaction failed - account id "
                << account_id << " balance is lower than " << amount << std::endl;
            pthread_mutex_unlock_secure(&log_mutex);
            return;
        }
        this->writeLock();
        this->balance-=amount;
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << atm_id << ": Account " << account_id << " new balance is " 
            << this->balance << " after " << amount << " $ was withdrawn" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
        this->writeUnlock();
    }
    else {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - password for account id " 
            << account_id << " is incorrect" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
        return;
    }
}

int Account::commission_acc(int percent) { // IMPORTANT: LOCK BEFORE AND AFTER USE IN BANK
    int amount_to_deduct = balance * percent/ 100;
    if(this->balance < amount_to_deduct){
        return 0;
    }
    balance -= amount_to_deduct;
    return amount_to_deduct;
}

int Account::get_balance_wo_lock() const {
        return balance;
}

int Account::get_balance_bi() { // get balance built in
    //this->readLock();
    int cached = balance;
    //this->readUnlock();
    return cached;
}

void Account::update_balance(int new_balance) {
    //this->writeLock();
    this->balance = new_balance;
    //this->writeUnlock();
}

