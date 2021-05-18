Projet de Systèmes d'Exploitations - Mathis Bouverot-Dupuis

Description générale du projet :
J'ai réalisé un début de système d'exploitation 32bits que je fais tourner avec QEMU (j'ai jamais essayé de le faire tourner sur une vraie machine). Dans l'état des choses mon OS ne "fait" rien de concret : je me suis arreté au milieu de l'implémentation du shell (j'avais prévu de faire quelques utilitaires comme ls, cd ou echo : echo est déjà là par exemple). J'ai implémenté une mini libc à ma façon (sans chercher à copier exactement la vraie libc), qui permet aux programmes utilisateurs d'utiliser des mechanismes fournis par l'OS comme des threads & processus, des structures de synchronisation (ex: locks), de lire/écrire dans des fichiers, etc. 
J'ai travaillé tout seul sur le projet donc j'ai malheureusement pas pu faire un système d'exploitation vraiment abouti, je me suis globalement concentré sur l'ordonnancement des processus/threads, avec pas mal de travail sur le système de fichiers aussi. Pour la mémoire virtuelle j'ai fait le minimum pour que ça tourne (en particulier je ne libère jamais de page et ne swap pas de page vers le disque dur). J'ai rien fait coté réseau.

Structure du projet:
-bootloader/ : mon propre bootloader (j'ai préféré en implémenter un que lire la doc de grub).
-kernel/ : tout ce qui est dans le ring 0 (noyau + drivers)
-libc/ : bibliothèque qui est compilée deux fois, d'une part je link avec le kernel, ça me donne une librairie libk qui est utilisée par le noyau, d'autre part je ne link pas (et donc j'utilise des system calls pour accéder au noyau) et ça me donne une librairie libc qui est pour les programmes utilisateurs.
-progs/ : les programmes utilitaires (cd, ls, xargs, etc.). J'ai pas eu le temps de beaucoup avancer là-dessus, en gros aucun ne marche (en particulier le Shell peut sembler marcher mais si on fait attention on voit qu'il y a encore des bugs).

Des détails sur le noyau :
-threads/processus : le modèle que j'ai adopté est basé sur des threads, qui sont des fils d'exécution de code (donc un thread=un code + un contexte d'éxecution pour pouvoir changer de threads à la volée). Un processus représente un ensemble de resources (descripteurs de fichiers, locks, address space), et contient plusieurs threads. Tous les threads au sein d'un processus partagent la meme mémoire et peuvent communiquer entre eux (avec wait() et join() et exit()). Un thread peut aussi manipuler son processus, avec process_exit(), process_fork(), exec(), etc.
-système de fichiers : je me sers du système de fichiers EXT2 (révision 0). Le disque dur initial est formatté en EXT2 (et j'ai rajouté le premier secteur de mon bootloader à l'adresse 0). Le reste du code du bootloader est stocké dans un fichier (c'est un 'flat binary'), et le noyau est aussi dans un fichier au format elf.

Quelques remarques :
J'ai fait l'erreur de compiler avec -O0 les premières semaines, et quand j'ai essayé de mettre les optimizations c'était trop tard, j'avais trop de bugs qui apparaissaient (j'ai passé beaucoup de temps à essayer de les résoudre : par exemple j'en ai résolu un vers la toute fin du projet : je me suis rendu compte qu'il fallait aligner %esp sur 16 bytes quand on appelle une fonction C depuis un code assembleur). Plutot que de passer une semaine à résoudre ces bugs je suis resté en -O0 et mon OS est donc très très lent.

De façon générale je me suis pas concentré sur implémenter chaque partie de l'OS de façon très efficace (à plusieurs endroits j'aurais pu utiliser des structures de données comme des tables de hashage ou des arbres de recherche, mais j'ai implémenté la solution la plus simple, souvent une liste chainée). J'ai plutot cherché à comprendre comme tout s'agence ensemble (c'est justement l'avantage d'un projet par rapport à un cours selon moi).

Je me suis rendu compte que faire des opérations sur les processus (e.g. fork, exec, exit) c'est horrible quand il y a plusieurs threads dans le processus (exemple : qu'est-ce qui se passe si un thread appelle fork() alors que d'autres threads dans le meme processus sont dans une section critique ?). Pour simplifier j'ai choisi d'interdire la plupart des opérations sur les processus quand il y a deux ou plus threads.

Je me suis inspiré notamment des sources suivantes :
Tutoriels généraux (pas toujours d'excellente qualité) :
http://www.brokenthorn.com/Resources/OSDevIndex.html
http://www.jamesmolloy.co.uk/tutorial_html/
Bootloader : 
http://independent-software.com/operating-system-development-first-and-second-stage-bootloaders.html/ 
Ext2 :
https://www.nongnu.org/ext2-doc/ext2.html#directory
Livres :
Operating Systems Principles & Practice (Anderson, Dahlin, volumes 1 & 2)
Understanding the Linux Kernel (Bovet, Cesati)
Et bien sur le site osdev, les manuels Intel et les références ELF, System V ABI, etc.