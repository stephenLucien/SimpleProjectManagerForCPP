#!/bin/bash
# \ref https://docs.github.com/en/authentication/troubleshooting-ssh/using-ssh-over-the-https-port

function setGithubPort() {
    local Port="$1"
    cat >>~/.ssh/config <<EOF
Host github.com
    Hostname ssh.github.com
    Port ${Port}
    User git
EOF
}

# test port 22
ssh -T -p 22 git@ssh.github.com

# test port 443
ssh -T -p 443 git@ssh.github.com
# setGithubPort 443
