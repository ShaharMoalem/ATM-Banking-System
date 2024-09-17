
# ATM-Banking-System
An implemented multi-threaded banking system in C++

## Description
This project implements a multi-threaded ATM banking system using POSIX `pthreads`, designed to simulate multiple ATMs performing concurrent transactions on shared bank accounts. The system supports operations such as withdrawals, deposits, and balance transfers between accounts, with each ATM functioning in parallel, handling operations independently. The project demonstrates my ability to design concurrent systems and apply synchronization mechanisms to ensure the integrity of shared data, with a focus on parallel programming and concurrency control.

## Technologies Used
- **Programming Languages:** C++
- **Libraries/Frameworks:** POSIX `pthreads`, Standard Template Library (STL) for data structures (e.g., `vector`, `map`)
- **Tools:** GDB for debugging, Makefile for build automation, Git for version control

## Features
- **Concurrent ATM Operations:** Each ATM is a separate thread, capable of processing transactions independently.
- **Synchronization Mechanisms:** Mutex locks are used to handle shared data safely between threads.
- **Logging:** Each ATM logs transactions to a shared log file with appropriate error handling for failed operations.
- **Transaction Types:** Supports a variety of operations, including:
  - Opening and closing bank accounts.
  - Depositing and withdrawing money.
  - Transferring funds between accounts.
- **Bank Commission System:** The bank periodically charges random commissions (1%-5%) from all accounts.
- **Real-Time Account Status:** Displays the current state of all accounts every 0.5 seconds.

## Getting Started

### Prerequisites
- C++ compiler (e.g., GCC)
- POSIX-compliant system (Linux or macOS)
  
### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/[YourGitHubUsername]/ATM-Banking-System.git
   cd ATM-Banking-System
   ```

2. Compile the program:
   ```bash
   make
   ```

### Usage
1. Prepare input files for each ATM, where each file contains a sequence of commands. For example:
   ```
   O 12345 1234 100
   W 12345 1234 50
   D 12345 1234 12
   ```

2. Run the program with input files for each ATM:
   ```bash
   ./ATM atm_input1.txt atm_input2.txt ...
   ```

### Commands Supported
- **Open Account:** `O <account> <password> <initial amount>`
- **Deposit:** `D <account> <password> <amount>`
- **Withdraw:** `W <account> <password> <amount>`
- **Balance Inquiry:** `B <account> <password>`
- **Close Account:** `Q <account> <password>`
- **Transfer:** `T <source account> <password> <target account> <amount>`

### ATM Behavior
- Each ATM processes one command every 1 second.
- ATMs wake up every 100 milliseconds, perform a transaction if available, and then go back to sleep.

### Bank Behavior
- The bank charges commissions (1%-5%) every 3 seconds.
- Prints the status of all accounts to the screen every 0.5 seconds.

### Example Log Output
```bash
ATM 1: New account id is 12345 with password 1234 and initial balance 100
ATM 1: Account 12345 new balance is 50 after 50 $ was withdrawn
Error ATM 1: Your transaction failed – account id 12345 balance is lower than 70
```

## Contributing
Contributions are welcome! Feel free to fork the repository, make your improvements, and submit a pull request.
```
