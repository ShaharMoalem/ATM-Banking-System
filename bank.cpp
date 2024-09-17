#include "bank.hpp"
#include <algorithm> // For std::any_of

//creates a vector of strings to hold each files name
std::vector<std::string> split_input(std::string const& input, const char delimiters) {
    std::vector<std::string> v;
    std::string target = input;
    size_t x = target.find(delimiters);
    while (x != std::string::npos) {
        v.push_back(target.substr(0, x));
        target = target.substr(x + 1);
        x = target.find(delimiters);
    }
    v.push_back(target); //the last part
    return v;
}

//picks a randum number betweeen 1 to 5
int Bank::random_one_to_five() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<int> dist5(1, 5); // Distribution in range [1, 5]
    return dist5(rng);
}

//find if the account with account id exists in the list
bool Bank::account_exists(int account_id) const{
        return std::any_of(acc_list.begin(), acc_list.end(),
          [account_id](const Account& acc) {
            return acc.getAccountId() == account_id;
        });
}

//return a pointer to the account in the list
Account* Bank::find_account(int account_id) {
        auto it = std::find_if(acc_list.begin(), acc_list.end(),
          [account_id](const Account& acc) {
            return acc.getAccountId() == account_id;
        });
        return it != acc_list.end() ? &(*it) : nullptr;
}

//sort the vector according to the account id- will be used every time we
//add an account- closing an account does not change the order of the accounts
void Bank::sort_acc_list() {
        std::sort(acc_list.begin(), acc_list.end(),
          [](const Account& a, const Account& b) {
            return a.getAccountId() < b.getAccountId();
        });
}

//ADT funcs
Bank::Bank() :profit(INITIAL_PROFIT), readers_num(INITIAL_NUMBER_OF_READERS){
    pthread_mutex_init_secure(&bread_mutex);
    pthread_mutex_init_secure(&bwrite_mutex);
}

Bank::~Bank() {
    pthread_mutex_destroy(&bread_mutex);
    pthread_mutex_destroy(&bwrite_mutex);
}
int rl1 = 0, rul1 =0, wl1=0,wul1=0;
void Bank::read_lock() {
    pthread_mutex_lock_secure(&bread_mutex);
    this->readers_num += 1;
    if (readers_num == 1) {
        this->write_lock();
    }
	/*pthread_mutex_lock_secure(&log_mutex);
	rl1++;
        output_file_log << "read locks "<< rl1 << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
    pthread_mutex_unlock_secure(&bread_mutex);
}
void Bank::read_unlock() {
    pthread_mutex_lock_secure(&bread_mutex);
    this->readers_num -= 1;
    if (readers_num == 0) {
        this->write_unlock();
    }
	/*pthread_mutex_lock_secure(&log_mutex);
	rul1++;
        output_file_log << "read unlocks "<< rul1 << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
    pthread_mutex_unlock_secure(&bread_mutex);
}
void Bank::write_lock() {
    pthread_mutex_lock_secure(&bwrite_mutex);
	/*pthread_mutex_lock_secure(&log_mutex);
		wl1++;
        output_file_log << "write locks " << wl1 << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
}
void Bank::write_unlock() {
    pthread_mutex_unlock_secure(&bwrite_mutex);
	/*pthread_mutex_lock_secure(&log_mutex);
		wul1++;
        output_file_log << "write unlocks " << wl1<<std::endl;
        pthread_mutex_unlock_secure(&log_mutex);*/
}

void Bank::open_account(int account_id, std::string input_pin, int initial_amount,int atm_id) {
    this->write_lock();
    if (this->account_exists(account_id)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - account with the same id exist" 
            << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
    else {
        Account new_account(account_id,initial_amount, input_pin);
        acc_list.push_back(new_account);
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << atm_id << ": New account id is " << account_id << " with password " 
            << input_pin << " and initial balance " << initial_amount << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
    this->write_unlock();
}

void Bank::close_account(int atm_id, int account_id, std::string input_pin) {
    this->write_lock();
    if (!this -> account_exists(account_id)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - account id " <<
            account_id << " does not exist" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
    else {
        Account* acc_p = find_account(account_id);
        if (acc_p) { 
            if (!acc_p->pin_compare(input_pin)) { // pin does not match
                pthread_mutex_lock_secure(&log_mutex);
                output_file_log << "Error " << atm_id << ": Your transaction failed - password for account id " 
                    << account_id << " is incorrect" << std::endl;
                pthread_mutex_unlock_secure(&log_mutex);
            }
            else {
                int old_balance = acc_p->get_balance_bi();
                auto it = std::find_if(acc_list.begin(), acc_list.end(), [account_id](const Account& acc) {
                        return acc.getAccountId() == account_id;
                    });
                if (it != acc_list.end()) {
                        acc_list.erase(it);
                }
                pthread_mutex_lock_secure(&log_mutex);
                output_file_log << atm_id << ": Account " << account_id << " is now closed. Balance was " 
                    << old_balance << std::endl;
                pthread_mutex_unlock_secure(&log_mutex);
            }
        }
    }
    this->write_unlock();
}

int Bank::get_profit() {
    int curr_profit = 0;
    curr_profit = this->profit;
    return curr_profit;
}

void Bank::deposit_account(int atm_id, int account_id, std::string input_pin, int amount) {
    this->read_lock();
    if (!this->account_exists(account_id)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - account id " << account_id << " does not exist" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
    else {
        Account* acc_p = find_account(account_id);
        if (acc_p) {
            acc_p->deposit(atm_id, account_id, input_pin, amount);
        }
    }
	this->read_unlock();
}

void Bank::withdraw_account(int atm_id, int account_id, std::string input_pin, int amount) {
    this->read_lock();
    if (!this->account_exists(account_id)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - account id " << account_id << " does not exist" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
		this->read_unlock();
		return;
    }
    else {
        Account* acc_p = find_account(account_id);
        if (acc_p) {
            acc_p->withdraw(atm_id, account_id, input_pin, amount);
        }
		this->read_unlock();
	}
}

void Bank::get_balance(int atm_id, int account_id, std::string input_pin) {
    this->read_lock();
    if (!this->account_exists(account_id)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - account id " << account_id 
            << " does not exist" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
    else {
        Account* acc_p = find_account(account_id);
        if (acc_p) { 
            acc_p->get_balance(atm_id, account_id, input_pin);
        }
    }
    this->read_unlock();
}

void Bank::transfer(int atm_id, int account_id, std::string input_pin, int target_id, int amount) {
	if(target_id == account_id){
				return;
	}
	if (!this->account_exists(account_id)) {
        pthread_mutex_lock_secure(&log_mutex);
        output_file_log << "Error " << atm_id << ": Your transaction failed - account id " << account_id << " does not exist" << std::endl;
        pthread_mutex_unlock_secure(&log_mutex);
    }
    this->read_lock();
    Account* acc_p = find_account(account_id);
    if (acc_p) { 
        if (!acc_p->pin_compare(input_pin)) { // pin does not match
            pthread_mutex_lock_secure(&log_mutex);
            output_file_log << "Error " << atm_id << ": Your transaction failed - password for account id " 
                << account_id << " is incorrect" << std::endl;
            pthread_mutex_unlock_secure(&log_mutex);
        }
        else if (acc_p->get_balance_bi() < amount) {
            pthread_mutex_lock_secure(&log_mutex);
            output_file_log << "Error " << atm_id << ": Your transaction failed - account id "
                << account_id << " balance is lower than " << amount << std::endl;
            pthread_mutex_unlock_secure(&log_mutex);
        }
        else {
            Account* tar_acc_p = find_account(target_id);
            if (tar_acc_p) {
				int tnew_balance = tar_acc_p->get_balance_bi() + amount;
				tar_acc_p->writeLock();
				tar_acc_p->update_balance(tnew_balance);
				tar_acc_p->writeUnlock();
				
				int acc_new_balance = acc_p->get_balance_bi() - amount;
				acc_p->writeLock();//account class function
				acc_p->update_balance(acc_new_balance);
				acc_p->writeUnlock();//account class function
				pthread_mutex_lock_secure(&log_mutex);
				output_file_log << atm_id << ": Transfer " << amount << " from account " << account_id <<
					" to account " << target_id << " new account balance is " << acc_new_balance 
						<< " new target account balance is " << tnew_balance << std::endl;
				pthread_mutex_unlock_secure(&log_mutex);
            }
        }
    }
    this->read_unlock();
}

void Bank::print_bank() {
    this->read_lock();
    for (auto& acc : this->acc_list) {
        acc.readLock();
    }
    printf("\033[2J\033[1;1HCurrent Bank Status\n");
    for (const auto& acc : this->acc_list) {
        int balance = acc.get_balance_wo_lock();
        std::string pin = acc.get_pin();
        std::cout << "Account " << acc.getAccountId() << ": Balance - " << balance 
            << " $, Account Password - " << pin << std::endl;
    }
    std::cout << "The Bank has " << this->get_profit() << " $\n";
    for (auto it = this->acc_list.rbegin(); it != this->acc_list.rend(); ++it) {
        it->readUnlock();
    }
    this->read_unlock();
}

void Bank::commission() {
	int percent = random_one_to_five();
	this->write_lock();
    for (auto& acc : this->acc_list) {
        acc.writeLock();
    }
    for (auto& acc : this->acc_list) {
        int gain = acc.commission_acc(percent);
		this->profit += gain;
		pthread_mutex_lock_secure(&log_mutex);
		output_file_log << "Bank: commissions of " << percent << " % were charged, bank gained "
			<< gain <<" from account "<<acc.getAccountId()<<std::endl;
		pthread_mutex_unlock_secure(&log_mutex);
    }
    for (auto it = this->acc_list.rbegin(); it != this->acc_list.rend(); ++it) {
        it->writeUnlock();
    }
    this->write_unlock();
}