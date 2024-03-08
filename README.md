# NAME

openpwd - store and retrieve passwords securely

# SYNOPSIS

    openpwd [ MODIFIERS ]... [ COMMANDS ]...

# DEPENDENCIES

xclip, dmenu, openssl(programa)

# DESCRIPTION

OpenPwd is a convenient and simple password manager that uses dmenu(1)
and xclip(1) to interact with the user. Passwords are saved in ~/.openpwd
by default, can be changed setting the environment variable OPENPWD_FILE.

It has the following extra features:

1. Everything is saved in a file, unlike pass(1) which saves one file
   per credential.

2. Usernames are encrypted, unlike pass(1).

3. The file size is always the same, this allows denying the existence
   of all the passwords. (Mr. Judge, I have forgotten the password, but
   I swear to you that the password file is empty).

4. A redundant salt can be added to the passwords, this allows to hide
   part of the password in your memory. (Mr. Judge, here is the password
   that decrypts the password database, the one that decrypts that file
   is not inside, sorry).

5. AES256 encryption is used.

6. Passwords never touch the disk decrypted, RAM is used at all times.

# SESSION TIMEOUT

The OpenPwd program does not leave any process in the background. The
session is saved in the user's shared memory.

To log out when there is no activity, for example in an hour of inactivity,
you *must* add the following task to your crontab(5) file.

    5 * * * * openpwd -c 3600 /clear

Read the cron(1) manual for more information.

# IMPORT PASS(1) PASSWORDS

Passwords can be imported from pass(1) with the following command:

    openpwd /import ~/.password-store

# INTERACTIVE MODE

Running the program without arguments runs it in interactive mode.

In interactive mode once the password file is unlocked the user is
presented with the list of saved credentials and the available
commands. Commands begin with "/".

If there is a URL (for example https://servicio.com/login) in the
clipboard the program first lists the credentials that contain the
domain, for example example "service.com/user1" and "service.com/user2".

Once a credential is selected the username is saved in the X selection
"primary" and the password in "clipboard".

You can access the user with the middle mouse button. To the password
with CTRL-P or paste.

The commands are the same as non-interactive mode.

# NON-INTERACTIVE MODE

- */select* : Ask user to select a credential.
- */user*   : Print username of selected credential.
- */pass*   : Print password of selected credential (with salt).
- */passq*  : Print password of selected credential (without salt, do not use).
- */delete*    : Delete selected credentials.
- */add K U P* : Add new credentials (Fail if it exists).
- */set K U P* : Add/replace credentials.
- */clear*      : Quit session.
- */import DIR* : Import passwords from pass(1).

When asked for a credential (with /select) the user always has the
possibility of typing the password raw, writting first a @ and then
the password.

# TODO

- Option to change the main password.
- Option to rename credentials.
- Export option.

# SEE ALSO

**OPENSSL(1)**, **XCLIP(1)**, **DMENU(1)**

# COLLABORATING

For making bug reports, feature requests and donations visit
one of the following links:

1. [gemini://harkadev.com/oss/](gemini://harkadev.com/oss/)
2. [https://harkadev.com/oss/](https://harkadev.com/oss/)
