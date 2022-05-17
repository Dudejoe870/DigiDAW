# DigiDAW Contribution Guidelines

## Project Structure
The project is split up into two main sections.
[Core](DigiDAWCore), and [UI](DigiDAWUI).

[Core](DigiDAWCore) is responsible for the actual 
audio processing engine and other internals, 
including the Mixer, VST Engine, Project File Format, 
Internal Track State, and more.

[UI](DigiDAWUI) is responsible for everything the 
user interacts with, program settings, and anything 
the GPU renders.

### SIMD
All SIMD code is within the [Core](DigiDAWCore) project, 
and is under [DigiDAWCore/priv_include/detail/simdhelper.h](DigiDAWCore/priv_include/detail/simdhelper.h), 
and is only used internally within the 
[Core](DigiDAWCore) project. 

It's important to keep all SIMD under this file, as later this 
file will probably be converted to use dynamic dispatching, 
for automatically using available SIMD extensions.

## Styling
This project uses C\++20, and thus, any C\++20 
features can be utilized. 
(I'd avoid overly abstracting everything, 
and excessively complicated templates unless absolutely neccessary 
and very well documented, however. 
That just makes the code less readable and maintainable)

### Source File Names
Filenames use snake_case, for example:
```
this_is_an_important_source_file.cpp
this_is_an_important_header_file.h
```
Header files use ```.h``` and not ```.hpp```.

### Function and Variable Names
Function names use PascalCase, while Variable names use camelCase. For example:
```C++
static unsigned int interestingVariable = 1337;
void DoSomething(int variousArguments, float andWhatNot);
```

Certain abbreviations can be uppercase, for example:
```C++
unsigned int variableLUFSThing = 3765;
int LUFSvariable = 5430;
void CalculateRMS(float amplitude, float& rms);
```
however, in Variable names, if the only word present 
is an abbreviation, then it's lowercase. 
Otherwise, it's uppercase. 
And if it's at the beggining of the name, 
the word after it is lowercase. Otherwise it follows camelCase rules, 
just with the abbreviation being uppercase.
See above for examples of all cases.

### Other Notes
Static classes (classes with only static members in them) should be used 
instead of namespaces, for example:

Use
```C++
// StaticHelpers.h
namespace My::Namespace::Path
{
    class StaticHelpers
    {
    public:
        static unsigned int someVariableOrWhatever;

        static void MyStaticFunction();
    };
}

// StaticHelpers.cpp
namespace My::Namespace::Path
{
    unsigned int StaticHelpers::someVariableOrWhatever = 5;

    void StaticHelpers::MyStaticFunction()
    {
        // Implementation Here
    }
}
```
Instead of
```C++
// StaticHelpers.h
namespace My::Namespace::Path::StaticHelpers
{
    extern unsigned int someVariableOrWhatever;

    void MyStaticFunction();
}

// StaticHelpers.cpp
namespace My::Namespace::Path
{
    unsigned int StaticHelpers::someVariableOrWhatever = 5;

    void StaticHelpers::MyStaticFunction()
    {
        // Implementation Here
    }
}
```
