# Star-Catalog-Assignment
Multithreaded C application that calculates average angular distance of stars in the Tycho Star Catalogue.

## What I learned
- C multithreading using pthreads  
- Mutex Locks to prevent race conditions
- Generic Functions  

## Description
In this assignment I was provided code that serially calculates the average angular distance between 50,000 stars in the Tycho Star Catalogue.  
This code, running serially, takes a significant amount of time to run.  
Using pthreads I multithreaded the program to increase it's efficiency at computing the angular distances.  

## Usage
Determine the amount of threads to use by adding a command line parameter -t and a value 2, 4, 10, 25, 100, and 1000 for calculating the minimum, maximum and mean angular distance on the codespace