# Basic use case instructions


## Getting started

 * To start using the QKey, plug it into a USB port and open a terminal program and point it to the USB serial port.
   * For windows a good option is [Tera Term](https://ttssh2.osdn.jp/)
   * For linux I like to use minicom
   
 * When you open the serial port you should get a prompt like this. Type in your master password and hit enter
   * _If you haven't yet used the QKey then type in a new password here_
 ```
 Serial connected
 Initializing SD card...done.
 Password:
 ```
 
 * After you type in your password some information will be printed. Confirm that the File check matches what you recall from previously and press the button if so. If it does not match abort and retype your password.
   * _Whenever you see the "Are you sure?(Button)" prompt you can hold the button for a couple seconds to abort._
```   
Records: 27
File check: JhbJ1wsqhnr-t99|
Are you sure?(Button)
```

 * At this point you will be greeted by the prompt `>`. Type `h` to print the list of supported commands

## Saving passwords

 * Log in as described above, then type `a` to Add a new password
 * Enter a description for the password. I recommend a description of the website, and a date so you know which passwords are getting old and should be changed.
 * Enter the username you need to log in to the website. For some websites this may be an email address; look at the website itself to figure out what to use.
 * Select `t` for Tab or `n` for Newline separator. For most websites this should be tab.
 * Enter the desired password. You have a choice of `r` for random or `m` for manual
   * Random is more secure, but requires you to change your password on the website
   * Manual allows you to enter an existing password
 * The password you entered will be printed for confirmation. I recommend copy-pasting this into the website login to confirm it is accepted before hitting the button to confirm or abort.
 
```
>
Add
Description: test
Username: foo
Separator(t,n): t
Random or Manual password?(r/m) r
Password: uUYBoozpK?b03G[o
Are you sure?(Button)
Ok
Added as record: 27
```

## Logging into websites


 * Log in as described above, then type `f` to find a password
   * Type the search term desired to list passwords with this search term in them
   * If you can't think of a good search term, hit `p` to list all of the entries
```
>
Find
Search term: test
27: test, foo, t
```
 * Type `e` to enter the password onto a website. When prompted type the index of the password you found(27 in this example)
 * Go to the website and clear out any existing username or password, then put you cursor in the username field
 * Push the button on your QKey to have it type in your username and password automatically
```
>
Enter(i)
Index: 27
27
test, foo, t
Are you sure?(Button)
Ok
```
