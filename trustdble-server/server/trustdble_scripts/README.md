# TrustDBle Development Scripts

This folder contains scripts that are executed as part of our CI pipeline or can be used for development.

## Constant values

To avoid the specification of the same variable in multiple scripts, more frequently needed variables are defined in the `.constants.sh` file.
This file is sourced into the `trustdble` script and because of the `export` command available in all scripts. 
To add a new global variable, simply add it to the `.constants.sh` file like

```bash
export MY_VAR=my_var_value
```

## Add a new script

Each script in this folder represents a command of the `trustdble` script in the projects root directory. The command name is specified by the script name, where underscore `_` is used as separator.

    trustdble_scripts/test_script.sh ==> ./trustdble.sh test script

To create a new script simply use `.template.sh` as a baseline.

    cp trustdble_scripts/.template.sh trustdble_scripts/your_script.sh

In your new command script you can then specify your arguments, help output and script logic.