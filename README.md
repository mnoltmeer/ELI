![card](https://github.com/user-attachments/assets/d6b63141-3610-4400-90aa-472aaf6a3680)

Project site: http://eli.cc.ua

The principle of operation: sequential interpretation

Form: dynamically linked library (DLL)

<h3>The Purpose</h3>

Extern Logic Interpreter was developed to extend logical 
manipulation outside the application code itself. Its direct purpose is to abstract the program as 
much as possible from logical operations that could be easily modified without recompiling the 
main program. Ideally, an application that uses ELI can consist only of implemented mechanisms 
designed to interact with the environment (e.g., features for working with files, outputting 
information to the screen, etc.), along with special wrapper functions that allow ELI to access these 
mechanisms. The interpreter itself will be responsible for linking all of this to a specific algorithm.

<h3>The Operating Mechanism</h3>

ELI is a line interpreter that runs commands with a syntax similar to high-level 
programming languages such as C++. Line translation is mostly performed sequentially, except for 
the user-defined function (UDF), conditions, and loops, for which the bodies are parsed separately 
before the main script translation starts. 
The unit of translation is a script. A script consists of lines separated by a “;” character. ELI 
can receive the script text directly from the main application or load it from an external file. There 
can be several translation units, their code is added to the translation using the #include directive or 
the Run() function. 
Before translation, the interpreter renders the script: it selects string constants and code 
snippets in the text in curly brackets “{}”. Code snippets are replaced in the script text with special 
identifiers, and their bodies get into a special stack. Such snippets are called deferred, and their 
translation starts only on a direct request.  

The script text may contain comments. A commented line must begin with two forward 
slash characters “//” and end with a semicolon “;”.

ELI provides the user with the following functionality:

- Directives: direct commands to the interpreter. 
- Variables: internal variables of the interpreter that exist during the script’s execution. Scope: 
the current context.  
- Simple and complex mathematical operations with numbers.  
- Loops.  
- Conditions. 
- Functions: pointers to wrapper functions from the main applicator. Enables the use of 
algorithms contained in the main application. The functions can pass the result of their work from 
the main application to the script. 
- Procedures: code snippets that are invoked (and translated) only when directly requested. 
As opposed to functions, they do not return values. 
- Objects: abstractions that represent a set of typed data and enable operating with them as a 
whole using properties and methods.
- Parameter stack: a layer between the main application and the ELI, used for data exchange. 
It is a dynamically changing set of typed structures that characterize the concept of “parameter”. 
- Message stack output: errors and service messages that occur during the translation process 
are saved to a special string variable that can be passed to the main application using the appropriate 
interpreted function. 
- Variable stack output: the contents of the variable stack are saved in a formatted form to a 
string variable. 
- Function stack output: the contents of the wrapper function stack are saved to a string 
variable. Not only the functions described in the interpreter library are saved, but also those stated 
in the main application. 
- The output of the parameter stack. 
- The output of the object stack. 
- Translation log: an option that is set when the script is launched. Enables logging of strings 
to a file and saves error messages there. Logging slows down the translation process. 
Interpreter log: an option set using a special interpreter function or the ELI::SetDebug() 
method. Enables logging of the ELI itself, and directs the output to a file or STDOUT. Logging 
significantly slows down the interpreter.

<h3>Implementation</h3>

ELI is written using C++, with std::vector<std::wstring> containers for storing script strings, 
as well as special classes that describe stacks of variables, parameters, functions, procedures, 
objects, and individual code snippets. Instances of these classes are included in the ELI class. The 
string encoding is ANSI. The internal language is essentially a wrapper over C++ code. Each string 
is interpreted into a set of C++ commands using a special algorithm and then executed. Using a 
special method of the ELI class, the application passes the script text (or the name of the file to be 
translated) to the interpreter, as well as an optional set of input parameters. The ELI interprets the 
strings and then returns the result of the process (type const wchar_t*), if necessary. In case of an 
error, it returns the constant string “-err-“. 
