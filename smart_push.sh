#!/bin/bash
# Smart script to push code to GitHub (handles first time, remote, branch, rebase, etc.)

# Colors
GREEN="\033[1;32m"
CYAN="\033[1;36m"
YELLOW="\033[1;33m"
RED="\033[1;31m"
RESET="\033[0m"

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}📦 Checking git repository...${RESET}"

if [ ! -d ".git" ]; then
    echo -e "${YELLOW}🛠 No git repo found. Initializing...${RESET}"
    git init
fi

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}🔍 Checking for remote 'origin'...${RESET}"

remote_url=$(git remote get-url origin 2>/dev/null)

if [ -z "$remote_url" ]; then
    echo -e "${YELLOW}❗ No remote 'origin' found.${RESET}"
    read -p "Enter GitHub repository URL (HTTPS recommended): " repo_url

    # Auto-convert SSH → HTTPS
    if [[ "$repo_url" =~ ^git@github\.com:(.*)\.git$ ]]; then
        repo_url="https://github.com/${BASH_REMATCH[1]}.git"
        echo -e "${GREEN}🔄 Converted SSH URL to HTTPS: $repo_url${RESET}"
    fi

    git remote add origin "$repo_url"
    echo -e "${GREEN}  Remote 'origin' added.${RESET}"
else
    echo -e "${GREEN}  Remote 'origin' found: $remote_url${RESET}"
fi

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}➕ Adding all files...${RESET}"
git add .

echo -e "${CYAN}---------------------------------------------${RESET}"
read -p "Enter commit message: " commit_msg
if [ -z "$commit_msg" ]; then
    commit_msg="Update $(date '+%Y-%m-%d %H:%M:%S')"
fi

if git diff --cached --quiet; then
    echo -e "${YELLOW}  Nothing to commit.${RESET}"
else
    echo -e "${GREEN}  Committing...${RESET}"
    git commit -m "$commit_msg"
fi

echo -e "${CYAN}---------------------------------------------${RESET}"

# Ensure branch is called main
current_branch=$(git symbolic-ref --short HEAD 2>/dev/null)
if [ "$current_branch" != "main" ]; then
    echo -e "${GREEN}🌿 Setting branch name to 'main'...${RESET}"
    git branch -M main
fi

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}⏬ Pulling latest changes from remote (with rebase)...${RESET}"
if git pull --rebase origin main; then
    echo -e "${GREEN}✅ Pull successful.${RESET}"
else
    echo -e "${RED}❌ Pull failed. Resolve conflicts manually and run 'git rebase --continue'.${RESET}"
    exit 1
fi

echo -e "${GREEN}🚀 Pushing to GitHub...${RESET}"
if git push origin main; then
    echo -e "${GREEN}✅ Push successful!${RESET}"
else
    echo -e "${RED}❌ Push failed. Please resolve issues and try again.${RESET}"
    exit 1
fi

echo -e "${CYAN}---------------------------------------------${RESET}"
echo -e "${GREEN}✅ All done!${RESET}"
