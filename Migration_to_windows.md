# Migration To Windows

## Why Migrate to Windows

I always developed my programs under the WSL but, for graphical projects, WSL doesn't work.
This project uses SDL3 to create the graphical interface so it was a matter of time until i needed to switch

## The Options

After thinking a bit, I had 2 ideas in mind:

1. Create a Linux VM and work from there
2. Set up Windows with all the tools necessary for development

This options don't seem that dificult to set up, i am fairly familiar with setting up a Linux VM and, in Windows, i could install VS Code or VS
as they come with everything i need.
The problem appeared when i wanted to use my NeoVim setup instead of VS.
This forced me to install a bunch of tools, mainly the compiler, git and cmake, in Windows which i don't have any experience how to do it

So after a bit more thinking, I went with the 2º option with NeoVim setup.

## The Process

I started by investigating how to install C/C++ compilers in Windows and i found the **MSYS2**.
So i follow their main page of installation and installed the main tools i needed (cmake, gcc, git)
This was very easy since the tool emulates a Linux enviroment so installing anything was a piece of cake

With everything installed, the next step is to put the binaries folder path (in my case **ucrt64/**) in my Windows env PATH variable
This enables the option to call all this tools in the Windows terminal

After that, i installed NeoVim for Windows, put my config in the **AppData\Local\nvim** and run **nvim** in the terminal.
And an error appeared saying it couldn't git clone LazyVim, because of ssl certificate **ca-bundle.crt**

TLDR: The **ca-bundle.crt** was a 0 bytes file. I needed to make a new one with the command **update-ca-trust**, but this command didn't work for me, so i used this one instead

```bash
p11-kit extract --format=pem-bundle \
  --filter=ca-anchors \
  /ucrt64/etc/ssl/certs/ca-bundle.crt
```

With the certificate problem solved, now i can work in Windows
