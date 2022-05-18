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
is under [DigiDAWCore/priv_include/detail/simdhelper.h](DigiDAWCore/priv_include/detail/simdhelper.h), 
and is only used internally within the 
[Core](DigiDAWCore) project. 

It's important to keep all SIMD under this file, as later this 
file will probably be converted to use dynamic dispatching, 
for automatically using available SIMD extensions.

## Styling
This project uses C\++20, and thus, any C\++20 
features can be utilized. 
(I'd avoid overly abstracting everything, 
and excessively complicated templates unless absolutely necessary 
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
Function names use PascalCase, while variable names use camelCase. For example:
```C++
static unsigned int interestingVariable = 1337;
void DoSomething(int variousArguments, float andWhatNot);
```

Certain abbreviations can be uppercase, but only in function names, for example:
```C++
unsigned int variableLufsThing = 3765;
int lufsVariable = 5430;
void CalculateRMS(float amplitude, float& rms);
```
try to prefer putting the abbreviation at the beginning of the variable name (in lowercase).

### ImGUI
Since ImGUI uses the Immediate Mode style of API, it's prefered 
to use scoping between Begin/End and Push/Pop blocks of code. 
For example:
```C++
void RenderSomething()
{
    ImGui::PushStyleVar(ImGuiStyleVar_Something, ImVec2(0.0f, 1.0f));
    {
        if (ImGui::Begin("Test Window"))
        {
            ImGui::BeginVertical("##random_layout");
            {
                ImGui::Text("Hello");
            }
            ImGui::EndVertical();
        }
        ImGui::End();
    }
    ImGui::PopStyleVar();
}
```

### Static Classes
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

### Namespaces
All namespaces should start with ```DigiDAW::Core``` 
if in the [Core](DigiDAWCore) project, 
or ```DigiDAW::UI``` if in the [UI](DigiDAWUI) project.

### Include Paths
Include paths should start with ```digidaw/core``` if in the 
[Core](DigiDAWCore) project, or ```digidaw/ui``` if in the 
[UI](DigiDAWUI) project. Unless the include path is to a 
private include (only applies to the [Core](DigiDAWCore) project)
or is a third-party library that warrants a specific include path.

### Comments
Comments can be multiline with either repeating ```//```, or using ```/* */``` (usually for explaining project architecture more than indvidual lines in functions).

Occasionally using ```/* */``` mid-line, when the comment pertains to a specific 
section of code in a single line is appropriate.
