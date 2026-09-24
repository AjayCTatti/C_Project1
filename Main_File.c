#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "Privacy_File.c"

#define MAX_TXNS 5
#define FILENAME "accounts.dat"
#define TOTAL_ACC "Total_accounts.dat"
#define INTEREST_INTERVAL (90LL * 24 * 60 * 60)

/* ------ Structure to store individual transaction data ------- */

typedef struct
{
    char type[15];
    double amount; // Deposit or Withdraw
    char date_time[30];
} Transaction;

/* ------- Strcture to store total accounts ------- */

typedef struct
{
    char name[50];
    char acc_no[12];
    char mobile_no[11];
} Total_acc;

/* ------ Structure to store individual accounts data ------ */

typedef struct
{
    char acc_no[12];
    char name[50];
    double balance;
    Transaction history[MAX_TXNS];
    char ifsc_code[6];
    char mobile_no[11];
    char acc_gen_date[30];
    time_t last_interest_dt;
    char acc_type[35];
    int txn_count;
} Account;

/* ------- Function declarations for every operations on  individual accounts ------ */

void create_account(void);
void deposit(void);
void withdraw(void);
void mini_statement(void);
void close_account(void);
void log_transaction(Account *acc, const char *type, double amount);
void dep_interest_amount(void);
void update_account(void);
void count_acc(const Account *acc);
void display_accounts(void);

/* ------- Helper functions ------- */

static void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

static int read_int(const char *prompt, int *value)
{
    printf("%s", prompt);
    if (scanf("%d", value) != 1)
    {
        clear_input_buffer();
        return 0;
    }
    clear_input_buffer();
    return 1;
}

static int read_double(const char *prompt, double *value)
{
    printf("%s", prompt);
    if (scanf("%lf", value) != 1)
    {
        clear_input_buffer();
        return 0;
    }
    clear_input_buffer();
    return 1;
}

static int find_account(FILE *fp, const char *target_acc, Account *acc)
{
    while (fread(acc, sizeof(Account), 1, fp) == 1)
    {
        if (strcmp(acc->acc_no, target_acc) == 0)
            return 1;
    }
    return 0;
}

/* ------- Actual code starts here ------- */

int main(void)
{
    char gen_captcha[6];
    char stored_pass[20] = "";
    char input_captcha[10];
    char input_pass[20];
    int choice;

    srand((unsigned int)time(NULL));

    while (1)
    {
        /* ------ Check if password file exists ------- */
        FILE *file = fopen(PASSWORD, "rb");

        if (file == NULL)
        {
            create_password();
            return 0;
        }

        if (fscanf(file, "%19s", stored_pass) != 1)
        {
            fclose(file);
            printf("Password file is empty or invalid.\n");
            create_password();
            return 0;
        }
        fclose(file);

        printf("\n============================ LOGIN PAGE ============================\n");
        printf("1. Password\n");
        printf("2. Forget Password\n");
        printf("3. Exit\n");

        if (!read_int("Enter your choice: ", &choice))
        {
            printf("Invalid input type!\n");
            continue;
        }

        switch (choice)
        {
        case 1:
            printf("Enter your password: ");
            scanf("%19s", input_pass);
            clear_input_buffer();

            printf("Enter the captcha: ");
            gencaptcha(6, gen_captcha);
            scanf("%9s", input_captcha);
            clear_input_buffer();

            if (strcmp(input_pass, stored_pass) != 0)
            {
                printf("Invalid password!\n");
                break;
            }

            if (strcmp(input_captcha, gen_captcha) != 0)
            {
                printf("Invalid captcha!\n");
                break;
            }

            while (1)
            {
                printf("\n========================= ____BANK ================================\n");
                printf("1. Create Account\n");
                printf("2. Deposit Money\n");
                printf("3. Withdraw Money\n");
                printf("4. Account Details / Mini Statement (Last 5 Txns)\n");
                printf("5. Close Account\n");
                printf("6. Update Account (Only Mobile No.)\n");
                printf("7. Deposit Interest Amount\n");
                printf("8. Display Total Present Accounts Details\n");
                printf("9. Exit\n");

                if (!read_int("Select an option: ", &choice))
                {
                    printf("Invalid input type!\n");
                    continue;
                }

                switch (choice)
                {
                case 1:
                    create_account();
                    break;
                case 2:
                    deposit();
                    break;
                case 3:
                    withdraw();
                    break;
                case 4:
                    mini_statement();
                    break;
                case 5:
                    close_account();
                    break;
                case 6:
                    update_account();
                    break;
                case 7:
                    dep_interest_amount();
                    break;
                case 8:
                    display_accounts();
                    break;
                case 9:
                    printf("Exiting the system. Goodbye!\n");
                    return 0;
                default:
                    printf("Invalid choice! Try again.\n");
                }
            }

        case 2:
            change_password();
            break;

        case 3:
            return 0;

        default:
            printf("There is no such choice.\n");
        }
    }
}

/* ------ Internal Helper:Logs a transaction inside the circular buffer array ------ */

void log_transaction(Account *acc, const char *type, double amount)
{
    time_t now = time(NULL);

    if (acc->txn_count < MAX_TXNS)
    {
        int index = acc->txn_count;
        strncpy(acc->history[index].type, type,
                sizeof(acc->history[index].type) - 1);
        acc->history[index].type[sizeof(acc->history[index].type) - 1] = '\0';
        acc->history[index].amount = amount;
        snprintf(acc->history[index].date_time,
                 sizeof(acc->history[index].date_time), "%s", ctime(&now));
        acc->txn_count++; // Always increment life counter
    }
    else
    {
        int i;

        for (i = 0; i < MAX_TXNS - 1; i++)
            acc->history[i] = acc->history[i + 1];

        strncpy(acc->history[MAX_TXNS - 1].type, type,
                sizeof(acc->history[MAX_TXNS - 1].type) - 1);
        acc->history[MAX_TXNS - 1].type[
            sizeof(acc->history[MAX_TXNS - 1].type) - 1] = '\0';
        acc->history[MAX_TXNS - 1].amount = amount;
        snprintf(acc->history[MAX_TXNS - 1].date_time,
                 sizeof(acc->history[MAX_TXNS - 1].date_time),
                 "%s", ctime(&now));
    }
}

/* ------ Counting and storing every account after creation of every account ------ */

void count_acc(const Account *acc)
{
    Total_acc total;
    FILE *fp = fopen(TOTAL_ACC, "ab"); // File Opened in append binary mode

    if (fp == NULL)
    {
        printf("Error opening total accounts file!\n");
        return;
    }

    memset(&total, 0, sizeof(total)); // Clear trash memory values
    strncpy(total.acc_no, acc->acc_no, sizeof(total.acc_no) - 1);
    strncpy(total.name, acc->name, sizeof(total.name) - 1);
    strncpy(total.mobile_no, acc->mobile_no, sizeof(total.mobile_no) - 1);

    fwrite(&total, sizeof(total), 1, fp);
    fclose(fp);
}

/* ------ Creating new account ------ */


void create_account(void)
{
    Account acc;
    FILE *fp;
    int choice;
    double initial_deposit;

    memset(&acc, 0, sizeof(acc)); // Clear trash memory values
    acc.last_interest_dt = time(NULL);

    printf("\nEnter Account Holder Name: ");
    scanf(" %49[^\n]", acc.name); // Scans text strings with spaces cleanly
    clear_input_buffer();

    acc_no_gen(11, acc.acc_no);

    printf("Your system generated Account No. (Please note it down): %s\n",
           acc.acc_no);

    printf("Enter Mobile No.: ");
    scanf("%10s", acc.mobile_no);
    clear_input_buffer();

    strcpy(acc.ifsc_code, "12345");
    strcpy(acc.acc_gen_date, __DATE__);

    printf("\nChoose the Account Type:\n");
    printf("1. Business Account\n");
    printf("2. Savings Account\n");

    if (!read_int("Enter choice: ", &choice))
    {
        printf("Invalid account type.\n");
        return;
    }

    if (choice == 1)
    {
        strcpy(acc.acc_type, "Business Acc");
        printf("Minimum balance must be 5000.\n");

        do
        {
            if (!read_double("Enter initial deposit: ", &initial_deposit))
            {
                printf("Invalid amount!\n");
                continue;
            }

            if (initial_deposit < 5000)
                printf("Amount is less than the minimum balance.\n");
        } while (initial_deposit < 5000);
    }
    else if (choice == 2)
    {
        strcpy(acc.acc_type, "Savings Acc");
        printf("Minimum balance must be 500.\n");

        do
        {
            if (!read_double("Enter initial deposit: ", &initial_deposit))
            {
                printf("Invalid amount!\n");
                continue;
            }

            if (initial_deposit < 500)
                printf("Amount is less than the minimum balance.\n");
        } while (initial_deposit < 500);
    }
    else
    {
        printf("Invalid account type. Account was not created.\n");
        return;
    }

    acc.balance = initial_deposit;
    acc.txn_count = 0;
    log_transaction(&acc, "Initial", acc.balance);

    fp = fopen(FILENAME, "ab"); // File Opened in append binary mode
    if (fp == NULL)
    {
        printf("Error opening database file!\n");
        return;
    }

    if (fwrite(&acc, sizeof(acc), 1, fp) != 1)
    {
        printf("Error saving account!\n");
        fclose(fp);
        return;
    }

    fclose(fp);
    count_acc(&acc);

    printf("Account successfully created and saved!\n");
}

/* ------ Deposition of Cash ------- */

void deposit(void)
{
    FILE *fp = fopen(FILENAME, "rb+"); // File opened in read / write binary mode
    Account acc;
    char target_acc[12];
    double amount;
    int found = 0;

    if (fp == NULL)
    {
        printf("No records found!\n");
        return;
    }

    printf("\nEnter Account Number: ");
    scanf("%11s", target_acc);
    clear_input_buffer();

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        if (strcmp(acc.acc_no, target_acc) == 0)
        {
            found = 1;

            printf("Account Holder: %s\n", acc.name);

            do
            {
                if (!read_double("Enter Amount to Deposit: ", &amount))
                {
                    printf("Invalid amount!\n");
                    continue;
                }

                if (amount <= 0)
                    printf("Deposit amount must be greater than zero.\n");
            } while (amount <= 0);

            acc.balance += amount;
            log_transaction(&acc, "Deposit", amount);
            // Move pointer backward 1 structure size to overwrite updated struct
            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            fflush(fp);

            printf("Deposit successful! New balance: %.2f\n", acc.balance);
            break;
        }
    }

    if (!found)
        printf("Account not found!\n");

    fclose(fp);
}

/* ------ Withdrawal of Cash ------- */

void withdraw(void)
{
    FILE *fp = fopen(FILENAME, "rb+"); // File opened in read / write binary mode
    Account acc;
    char target_acc[12];
    double amount;
    int found = 0;

    if (fp == NULL)
    {
        printf("No records found!\n");
        return;
    }

    printf("\nEnter Account Number: ");
    scanf("%11s", target_acc);
    clear_input_buffer();

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        if (strcmp(acc.acc_no, target_acc) == 0)
        {
            found = 1;

            printf("Account Holder: %s\n", acc.name);
            printf("Account Type: %s\n", acc.acc_type);
            printf("Current balance: %.2f\n", acc.balance);

            do
            {
                if (!read_double("Enter Amount to Withdraw: ", &amount))
                {
                    printf("Invalid amount!\n");
                    continue;
                }

                if (amount <= 0 || amount > acc.balance)
                    printf("Invalid amount or insufficient balance!\n");
            } while (amount <= 0 || amount > acc.balance);

            acc.balance -= amount;
            log_transaction(&acc, "Withdraw", amount);

            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            // Move pointer backward 1 structure size to overwrite updated struct
            fwrite(&acc, sizeof(acc), 1, fp);
            fflush(fp);

            printf("Withdrawal successful! Remaining balance: %.2f\n",
                   acc.balance);
            break;
        }
    }

    if (!found)
        printf("Account not found!\n");

    fclose(fp);
}

/* ------- Account details / Mini statement up to 5 transactions ------- */

void mini_statement(void)
{
    FILE *fp = fopen(FILENAME, "rb"); // File opened in read binary mode
    Account acc;
    char target_acc[12];
    int found = 0;

    if (fp == NULL)
    {
        printf("No records found!\n");
        return;
    }

    printf("\nEnter Account Number: ");
    scanf("%11s", target_acc);
    clear_input_buffer();

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        if (strcmp(acc.acc_no, target_acc) == 0)
        {
            int total_printed;
            int i;

            found = 1;

            printf("\n==================================================================\n");
            printf("                  ACCOUNT DETAILS / MINI STATEMENT\n");
            printf("==================================================================\n");
            printf("Holder Name       : %s\n", acc.name);
            printf("IFSC Code         : %s\n", acc.ifsc_code);
            printf("Account No.       : %s\n", acc.acc_no);
            printf("Account Gen. Date : %s\n", acc.acc_gen_date);
            printf("Account Type      : %s\n", acc.acc_type);
            printf("Mobile No.        : %s\n", acc.mobile_no);
            printf("Branch            : Gadag-Betageri\n");
            printf("------------------------------------------------------------------\n");
            printf("%-15s %-12s %-25s\n", "Txn Type", "Amount", "Date & Time");
            printf("------------------------------------------------------------------\n");
            // Logic to print up to 5 transactions chronologically
            total_printed = (acc.txn_count < MAX_TXNS)
                                ? acc.txn_count
                                : MAX_TXNS;

            for (i = 0; i < total_printed; i++)
            {
                printf("%-15s %-12.2f %-25s",
                       acc.history[i].type,
                       acc.history[i].amount,
                       acc.history[i].date_time);
            }

            printf("------------------------------------------------------------------\n");
            printf("Final Current Balance: %.2f\n", acc.balance);
            printf("==================================================================\n");
            break;
        }
    }

    if (!found)
        printf("Account not found!\n");

    fclose(fp);
}

/* ------- Interest amount deposition for every 3 months ------- */

void dep_interest_amount(void)
{
    FILE *file = fopen(PASSWORD, "rb"); // File opened in read binary mode
    char stored_pass[20];
    char input_pass[20];
    Account acc;
    time_t now = time(NULL);
    int count = 0;

    if (file == NULL)
    {
        printf("Error opening password file!\n");
        return;
    }

    if (fscanf(file, "%19s", stored_pass) != 1)
    {
        fclose(file);
        printf("Invalid password file!\n");
        return;
    }
    fclose(file);

    printf("\nEnter the password for confirmation: ");
    scanf("%19s", input_pass);
    clear_input_buffer();

    if (strcmp(stored_pass, input_pass) != 0)
    {
        printf("Invalid password! No interest was deposited.\n");
        return;
    }

    FILE *fp = fopen(FILENAME, "rb+"); // File opened in read / write binary mode
    if (fp == NULL)
    {
        printf("No records found!\n");
        return;
    }

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        double amount;
        long long seconds_passed =
            (long long)difftime(now, acc.last_interest_dt);

        if (seconds_passed >= INTEREST_INTERVAL)
        {
            amount = acc.balance * 0.00625;
            acc.balance += amount;
            log_transaction(&acc, "Int_Amount", amount);
            acc.last_interest_dt = now;

            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            /* ------ When switching between reading andwriting on a same file stream opened for update ------- */
            fflush(fp);

            count++;
        }
    }

    fclose(fp);

    printf("\nInterest deposited successfully to %d account(s).\n", count);
}

/* ------- Updating Mobile number only ------- */


void update_account(void)
{
    FILE *fp = fopen(FILENAME, "rb+"); // File opened in read / write binary mode
    Account acc;
    char target_acc[12];
    char mobile_no[11];
    char acc_no[12];
    int found = 0;

    if (fp == NULL)
    {
        printf("No records found!\n");
        return;
    }

    printf("\nEnter Account Number: ");
    scanf("%11s", target_acc);
    clear_input_buffer();

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        if (strcmp(acc.acc_no, target_acc) == 0)
        {
            found = 1;
            strcpy(acc_no, acc.acc_no);

            printf("Enter your New Mobile No.: ");
            scanf("%10s", mobile_no);
            clear_input_buffer();

            strcpy(acc.mobile_no, mobile_no);

            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            fflush(fp);

            printf("Mobile No. updated successfully!\n");
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        printf("Account not found!\n");
        return;
    }
 
     /* ------- Updating Mobile number in Total_account.dat file also ------- */

    {
        FILE *total_fp = fopen(TOTAL_ACC, "rb+");
        Total_acc total;

        if (total_fp == NULL)
        {
            printf("Error opening total accounts file!\n");
            return;
        }

        while (fread(&total, sizeof(total), 1, total_fp) == 1)
        {
            if (strcmp(total.acc_no, acc_no) == 0)
            {
                strcpy(total.mobile_no, mobile_no);
                fseek(total_fp, -(long)sizeof(total), SEEK_CUR);
                fwrite(&total, sizeof(total), 1, total_fp);
                fflush(total_fp);
                break;
            }
        }

        fclose(total_fp);
    }
}

/* ------- Closing of account if balance is 0. Otherwise account cannot be closed ------- */

void close_account(void)
{
    FILE *fp = fopen(FILENAME, "rb"); // File opened in read binary mode
    FILE *temp = NULL;
    Account acc;
    char target_acc[12];
    char acc_no[12] = "";
    double amount = 0;
    int found = 0;
    int closed = 0;

    if (fp == NULL)
    {
        printf("No records found!\n");
        return;
    }

    temp = fopen("Temp.dat", "wb");
    if (temp == NULL)
    {
        fclose(fp);
        printf("Error opening temporary file!\n");
        return;
    }

    printf("\nEnter Account Number: ");
    scanf("%11s", target_acc);
    clear_input_buffer();

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        if (strcmp(acc.acc_no, target_acc) == 0)
        {
            found = 1;
            amount = acc.balance;
            strcpy(acc_no, acc.acc_no);

            if (acc.balance == 0.0)
            {
                closed = 1;
                continue;
            }
        }

        if (fwrite(&acc, sizeof(acc), 1, temp) != 1)
        {
            printf("Error writing temporary account file!\n");
            fclose(fp);
            fclose(temp);
            remove("Temp.dat");
            return;
        }
    }

    fclose(fp);
    fclose(temp);

    if (!found)
    {
        remove("Temp.dat");
        printf("Account not found!\n");
        return;
    }

    if (!closed)
    {
        remove("Temp.dat");
        printf("Account has balance: %.2f.\n", amount);
        printf("Account cannot be closed. Balance must be 0.\n");
        return;
    }

    if (remove(FILENAME) != 0 || rename("Temp.dat", FILENAME) != 0)
    {
        printf("Error updating account database. Account was not closed safely.\n");
        return;
    }

    /* ------- Closing of account in Total_accounts.dat file also. Only if balance is 0 ------- */

    {
        FILE *total_fp = fopen(TOTAL_ACC, "rb"); // File opened in read binary mode
        FILE *total_temp = NULL;
        Total_acc total;

        if (total_fp == NULL)
        {
            printf("Account closed, but total accounts file could not be updated.\n");
            return;
        }

        total_temp = fopen("Total_temp.dat", "wb");
        if (total_temp == NULL)
        {
            fclose(total_fp);
            printf("Account closed, but total accounts file could not be updated.\n");
            return;
        }

        while (fread(&total, sizeof(total), 1, total_fp) == 1)
        {
            if (strcmp(total.acc_no, acc_no) != 0)
                fwrite(&total, sizeof(total), 1, total_temp);
        }

        fclose(total_fp);
        fclose(total_temp);

        if (remove(TOTAL_ACC) != 0 || rename("Total_temp.dat", TOTAL_ACC) != 0)
        {
            printf("Account closed, but total accounts file could not be updated.\n");
            return;
        }
    }

    printf("Account closed successfully!\n");
}

/* ------- Displaying total active accounts ------- */

void display_accounts(void)
{
    FILE *fp = fopen(TOTAL_ACC, "rb"); // File opened in read binary mode
    Total_acc acc;
    int count = 0;

    if (fp == NULL)
    {
        printf("No accounts found!\n");
        return;
    }

    printf("\n%-34s %-19s %-19s\n",
           "Account Holder Name", "Account No.", "Mobile No.");
    printf("------------------------------------------------------------------\n");

    while (fread(&acc, sizeof(acc), 1, fp) == 1)
    {
        printf("%-34s %-19s %-19s\n",
               acc.name, acc.acc_no, acc.mobile_no);
        count++;
    }

    printf("------------------------------------------------------------------\n");
    printf("Total number of Accounts: %d\n", count);

    fclose(fp);
}
