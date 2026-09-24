#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define PASSWORD "password.dat"

/* Create a new password. */

void create_password(void)
{
    char pass[20];
    FILE *file;

    printf("Create new password: ");
    scanf("%19s", pass);

    file = fopen(PASSWORD, "wb");
    if (file == NULL)
    {
        printf("Error opening password file!\n");
        return;
    }

    fprintf(file, "%s", pass);
    fclose(file);

    printf("Password saved successfully!\n");
}

/* -------- Change the existing password.--------- */

void change_password(void)
{
    char new_pass[20];
    FILE *file;

    printf("Enter your new password: ");
    scanf("%19s", new_pass);

    file = fopen(PASSWORD, "wb");
    if (file == NULL)
    {
        printf("Error opening password file!\n");
        return;
    }

    fprintf(file, "%s", new_pass);
    fclose(file);

    printf("Password updated successfully!\n");
}

/* ------- Generate an alphanumeric captcha. -------- */

char *gencaptcha(unsigned int length, char gen_captcha[])
{
    static const char characters[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";

    unsigned int i;

    for (i = 0; i < length; i++)
        gen_captcha[i] = characters[rand() % (sizeof(characters) - 1)];

    gen_captcha[length] = '\0';

    printf("Captcha: %s\n", gen_captcha);
    return gen_captcha;
}

/* ------- Generate an account number containing digits only. ------- */

char *acc_no_gen(unsigned int length, char gen_acc_no[])
{
    unsigned int i;

    if (length == 0)
    {
        gen_acc_no[0] = '\0';
        return gen_acc_no;
    }

    for (i = 0; i < length; i++)
        gen_acc_no[i] = (char)('0' + rand() % 10);

    gen_acc_no[length] = '\0';

    return gen_acc_no;
}
