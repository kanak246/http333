# http333 - Web Server

This project is a multithreaded HTTP web server written in C++. It supports file serving and a search engine built on precomputed inverted indices.

## Live in Codespaces

> This repo is fully configured to run in GitHub Codespaces with Docker!

### To run the server:

1. **Click "Code" > "Codespaces" > "Create codespace on master"**
2. Wait ~1–2 mins for setup (Docker + `make` auto-run)
3. You’ll see **Port 8000** appear and auto-open in the browser
4. You’re ready to go! Start searching or navigate to static files.

---

## Manual Run (Locally or in Codespace Terminal)

```bash
make
./http333d 8000 projdocs unit_test_indices/*

