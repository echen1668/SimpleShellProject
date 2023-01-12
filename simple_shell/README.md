Eric Chen
echen5@umbc.edu
LD19338

My approach to creating my own shell is by creating a main function called shell() which contains a while loop executing a series of functions to make a functional shell that will exit by a return statement with exit code.

Part 1:

The way I handle arguments at execution, I have my code check if the number of arguments is greater than 1, the 1 being “./simple_shell” if yes, then an error message is outputted and shell() is not called and the program exits, if not, then shell() called.
 

My program presents a prompt, "Enter your command >>>> " in which the user should enter their command next to it.

My function shell_read_line() reads the user input command and put it in a dynamically allocated char which can be reallocated if needed if the command is too big and returns it.

My function shell_parse() wil parse the command from shell_read_line() by first splitting the string by spaces and then using unescape() function to delimit each substring by escape sequences and quotes.

I created a function called shell_exit() that will act as a built-in function for exit. It is called if the user input first argument is “exit” and the function will check if a seconds argument exists, if so, then it will check if it is a valid number. Exits if so or no second argument.

I free up all dynamically allocated memory right before I exit and I have a function that free each argument in the arguments array that is returned by shell_parse().


Part 2:

My proc()  function is called if the user input’s first argument is “proc”. proc() will first check if a second argument is passed after “proc”. If so, then proc() will merge a string “/proc/” and the second argument together and pass it in fopen() to open the proc file. If succeed, it will read the file character by character and print it out. 
