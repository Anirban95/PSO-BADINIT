#!/bin/bash Smart script to push code to GitHub (handles first time, remote, etc.)

# Colors
GREEN="\033[1;32m"
CYAN="\033[1;36m"
RESET="\033[0m"

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}📦 Checking git repository...${RESET}"

if [ ! -d ".git" ]; then
    echo -e "${GREEN}🛠 No git repo found. Initializing...${RESET}"
    git init
fi

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}🔍 Checking for remote 'origin'...${RESET}"

remote_url=$(git remote get-url origin 2>/dev/null)

if [ -z "$remote_url" ]; then
    echo -e "${GREEN}❗ No remote 'origin' found.${RESET}"
    read -p "Enter GitHub repository URL (e.g., https://github.com/user/repo.git): " repo_url
    git remote add origin "$repo_url"
    echo -e "${GREEN}  Remote 'origin' added.${RESET}"
else
    echo -e "${GREEN}  Remote 'origin' found: $remote_url${RESET}"
fi

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN} Adding all files...${RESET}"
git add .

echo -e "${CYAN}---------------------------------------------${RESET}"
read -p "Enter commit message: " commit_msg

echo -e "${GREEN}  Committing...${RESET}"
git commit -m "$commit_msg"

echo -e "${CYAN}---------------------------------------------${RESET}"

# Ensure branch is called main
current_branch=$(git symbolic-ref --short HEAD 2>/dev/null)

if [ "$current_branch" != "main" ]; then
    echo -e "${GREEN} Setting branch name to 'main'...${RESET}"
    git branch -M main
fi

echo -e "${GREEN} Pushing to GitHub...${RESET}"
git push -u origin main

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}  All done!${RESET}"
