## http333 - Web Server

In this project, the web server is given a set of .idx files (precomputed inverted indices), which are stored in the projdocs directory. The search engine reads these indices to find files that contain the user’s search terms and returns results with:

A list of matching files/websites
The number of times each word appears in each file
Clickable links to access the relevant documents
This enables fast search functionality over large document sets.

## Features
-  Multithreaded Request Handling – Uses a thread pool for efficient request processing
- Static File Serving – Supports serving HTML, CSS, JavaScript, images, and text files
- Search Engine Integration – Finds documents containing user search terms
- Ranked Search Results – Displays word counts and links to relevant files/websites
- Graceful Shutdown Support – Can be stopped using the /quitquitquit endpoint
## Live in Codespaces

This repo is fully configured to run in GitHub Codespaces with Docker!
Quick Start (Recommended)
Click "Code" > "Codespaces" > "Create codespace on master"
Wait ~1–2 minutes for setup (Docker + make auto-run)
You’ll see Port 8000 appear and auto-open in the browser
Start using the web server: navigate to static files or enter search queries##

###  Manual Run (Locally or in Codespace Terminal)

If you prefer to run the project manually:

make 
./http333d 8000 projdocs unit_test_indices/*
8000 – The port number the server will listen on (might need to make it public)

projdocs – The directory containing static files and index files

unit_test_indices/* – The precomputed search indices used for querying
To search for words:

Open your browser and visit:
http://localhost:8000
Enter a search term in the box
The results will display:
Files containing the term
The number of times the term appears
Clickable links to view the files


## Dependencies

This project requires:

Boost C++ Libraries (libboost-all-dev)
Google Test (optional, for unit testing)
In GitHub Codespaces, these dependencies should already be installed. If missing, install them using:

sudo apt update && sudo apt install -y build-essential g++ make libboost-all-dev

## Running Tests (Optional)

The test cases are currently disabled in the Makefile to avoid unnecessary dependencies.

To enable tests:

Uncomment the test_suite target in the Makefile
Run:
make clean
make test_suite
./test_suite
If Google Test is missing, install it first:

sudo apt install -y libgtest-dev cmake
cd /usr/src/googletest
cmake .
make
sudo cp lib/*.a /usr/lib/

