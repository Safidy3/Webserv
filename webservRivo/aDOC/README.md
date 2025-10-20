Voici la traduction en français du texte que tu m’as donné :

Partie obligatoire
Nom du programme
webserv

Fichiers à rendre
Makefile, *.{h, hpp}, *.cpp, *.tpp, *.ipp, fichiers de configuration

Makefile
NAME, all, clean, fclean, re

Arguments
[Un fichier de configuration]

Fonctions externes
Toutes les fonctionnalités doivent être implémentées en C++ 98.
execve, pipe, strerror, gai_strerror, errno, dup, dup2, fork, socketpair, htons, htonl, ntohs, ntohl, select, poll, epoll (epoll_create, epoll_ctl, epoll_wait), kqueue (kqueue, kevent), socket, accept, listen, send, recv, chdir, bind, connect, getaddrinfo, freeaddrinfo, setsockopt, getsockname, getprotobyname, fcntl, close, read, write, waitpid, kill, signal, access, stat, open, opendir, readdir et closedir.

Libft autorisée
n/a

Description
Un serveur HTTP en C++ 98

Vous devez écrire un serveur HTTP en C++ 98.
Votre exécutable doit être lancé de la manière suivante :

./webserv [fichier_de_configuration]
Même si poll() est mentionné dans le sujet et la grille d’évaluation, vous pouvez utiliser n’importe quelle fonction équivalente comme select(), kqueue() ou epoll().

Veuillez lire les RFC définissant le protocole HTTP, et effectuer des tests avec telnet et NGINX avant de commencer ce projet.
Bien que vous ne soyez pas obligé d’implémenter la totalité des RFC, leur lecture vous aidera à développer les fonctionnalités demandées.
HTTP 1.0 est suggéré comme point de référence, mais non imposé.

III.1 Exigences
Votre programme doit utiliser un fichier de configuration fourni en argument de la ligne de commande, ou disponible dans un chemin par défaut.

Vous ne pouvez pas exécuter un autre serveur web via execve.

Votre serveur doit rester non bloquant en permanence et gérer correctement les déconnexions des clients.

Il doit être non bloquant et n’utiliser qu’un seul poll() (ou équivalent) pour toutes les opérations d’E/S entre les clients et le serveur (listen inclus).

poll() (ou équivalent) doit surveiller simultanément la lecture et l’écriture.

Vous ne devez jamais faire un read ou un write sans passer par poll() (ou équivalent).

Vérifier la valeur de errno pour ajuster le comportement du serveur est strictement interdit après un read ou un write.

Vous n’êtes pas obligé d’utiliser poll() (ou équivalent) avant read() pour lire votre fichier de configuration.

⚠️ Comme vous devez utiliser des descripteurs de fichiers non bloquants, il est possible d’utiliser read/recv ou write/send sans poll(), et le serveur ne bloquera pas. Mais cela consommera plus de ressources système.
👉 Si vous tentez de faire un read/recv ou un write/send sur un descripteur de fichier sans poll() (ou équivalent), vous aurez 0.

Avec poll() (ou équivalent), vous pouvez utiliser toutes les macros ou fonctions associées (par ex. FD_SET pour select()).

Une requête à votre serveur ne doit jamais rester en attente indéfiniment.

Votre serveur doit être compatible avec les navigateurs web standards de votre choix.

Vous pouvez utiliser NGINX pour comparer les en-têtes et comportements de réponse (attention aux différences entre versions HTTP).

Vos codes de statut HTTP doivent être corrects.

Votre serveur doit avoir des pages d’erreur par défaut si aucune n’est fournie.

Vous ne pouvez pas utiliser fork sauf pour le CGI (PHP, Python, etc.).

Vous devez pouvoir servir un site statique complet.

Les clients doivent pouvoir uploader des fichiers.

Vous devez implémenter au minimum les méthodes GET, POST et DELETE.

Testez votre serveur sous charge pour garantir sa disponibilité.

Votre serveur doit pouvoir écouter sur plusieurs ports afin de servir différents contenus (cf. fichier de configuration).

Le projet ne couvre qu’un sous-ensemble des RFC HTTP.
Dans ce cadre, la fonctionnalité de virtual host est considérée hors sujet, mais vous pouvez l’implémenter si vous le souhaitez.

III.2 Spécificités MacOS
Comme macOS gère write() différemment des autres systèmes Unix, vous êtes autorisé à utiliser fcntl().
Les descripteurs doivent être en mode non bloquant pour avoir un comportement similaire à celui des autres OS Unix.
Vous pouvez utiliser fcntl() uniquement avec les flags suivants :

F_SETFL

O_NONBLOCK

FD_CLOEXEC
Tout autre flag est interdit.

III.3 Fichier de configuration
Vous pouvez vous inspirer de la section server des fichiers de configuration NGINX.

Dans le fichier de configuration, vous devez pouvoir :

Définir toutes les paires interface:port sur lesquelles votre serveur écoutera (permettant de servir plusieurs sites).

Définir des pages d’erreur par défaut.

Fixer la taille maximale autorisée pour le corps des requêtes client.

Spécifier des règles ou configurations par URL/route (pas besoin de regex), parmi :

Liste des méthodes HTTP acceptées pour la route.

Redirections HTTP.

Répertoire où chercher le fichier demandé (par ex. si l’URL /kapouet pointe sur /tmp/www/, alors /kapouet/pouic/toto/pouet sera cherché dans /tmp/www/pouic/toto/pouet).

Activation/désactivation du listing de répertoire.

Fichier par défaut à servir si la ressource demandée est un dossier.

Autoriser l’upload de fichiers côté client et définir l’emplacement de stockage.

Exécution de CGI selon l’extension de fichier (par ex. .php).

👉 À propos du CGI :

Informez-vous sur les variables d’environnement dans la communication serveur web ↔ CGI.

La requête et les arguments du client doivent être disponibles pour le CGI.

Pour les requêtes chunkées, votre serveur doit les dé-chunker avant de les envoyer au CGI (le CGI attend un EOF comme fin du body).

Même chose pour la sortie du CGI : si aucun content_length n’est fourni, EOF marque la fin des données.

Le CGI doit être exécuté dans le bon répertoire pour accéder aux chemins relatifs.

Votre serveur doit supporter au moins un CGI (php-CGI, Python, etc.).

Vous devez fournir des fichiers de configuration et des fichiers par défaut pour tester et démontrer toutes les fonctionnalités pendant l’évaluation.

Vous pouvez ajouter d’autres règles ou infos (comme un server_name si vous implémentez des virtual hosts).

Si vous avez un doute sur un comportement spécifique, comparez avec NGINX.

Un petit outil de test est fourni. Son usage n’est pas obligatoire si tout fonctionne avec vos tests et navigateurs, mais il peut vous aider à corriger des bugs.

⚠️ La résilience est primordiale : votre serveur doit toujours rester opérationnel.

Ne testez pas avec un seul programme. Écrivez vos tests dans un langage adapté (Python, Golang, voire C ou C++ si vous préférez).

Veux-tu que je mette cette traduction en format tableau bilingue (anglais | français) pour faciliter la comparaison, ou tu préfères la version seulement en français comme ici ?



No file chosenNo file chosen
ChatGPT can make mistakes. Check important info.


🔴 1. Gestion correcte des chemins root / location

Tu as du 404 car les chemins sont mal combinés (root + URI).
✅ À corriger avec soit alias, soit correction du root.

👉 Ce n'est pas encore conforme au sujet.

🔴 2. Gestion complète des 3 méthodes HTTP

Tu dois absolument :

✅ GET → OK
⚠️ POST → Pas encore géré correctement

Pour POST, tu dois :

Lire le body via Content-Length ou chunked.

Autoriser l’upload dans certaines locations.

Stocker dans un dossier défini (ex: /uploads)

❌ DELETE → à faire !

Supprimer un fichier si présent

Retourner 200 / 204 / 404 selon le cas

🔴 3. Upload de fichiers

⚠️ Pas encore implémenté :

“Clients must be able to upload files.”

Faut :

Lire le body

Respecter client_max_body_size

Enregistrer dans upload_path

🔴 4. Gestion correcte des codes d’erreur :

Tu utilises :

error_page 404 /errors/404.html;
error_page 500 /errors/500.html;


Il faut :

Servir ces fichiers correctement

Retourner le bon header (404 Not Found)

🔴 5. Autoindex

Tu as :

autoindex off;


Mais il faut :

Supporter autoindex on;

Générer un listing si activé

🔴 6. Timeout / Connexions fermées proprement

Le sujet impose :

“A request to your server should never hang indefinitely.”

⚠️ Pas encore de timeout sur clients.
⚠️ Pas de gestion de POLLHUP ou POLLERR.

🔴 7. Pas de lecture/écriture directe sans poll()

Dans handleClientData tu fais :

recv(...)
send(...)


Mais tu ne vérifies pas POLLOUT, donc ⚠️ non conforme.

🔴 8. CGI incomplet

Tu as :

cgi_extension .php;
cgi_path /usr/bin/php-cgi;


Mais côté serveur :

Lancement du CGI (fork + execve) → non fait

Env vars (REQUEST_METHOD, CONTENT_LENGTH…) → NON

Gestion body → NON

Retour CGI au client → NON

🔴 9. Redirections HTTP

Le sujet impose :

http redirection


Pas encore supporté (return 301/302)

🔴 10. Directory par défaut si URI = dossier

Tu dois :

Chercher index.html OU

Générer l’autoindex si activé

✅ Résumé final : ce qu’il te reste à faire
🎯 CORE À FAIRE

✅ Corriger root/location pour éviter les 404

✅ Finir GET (avec autoindex + index fallback)

🔴 Implémenter POST (body / upload / Content-Length / chunked)

🔴 Implémenter DELETE

🔴 Implémenter CGI correctement (fork/exec/environ)

🔴 Gérer POLLOUT comme POLLIN dans poll

🔴 Timeout + déconnexion clients propres

🔴 Error pages dynamiques

🔴 HTTP redirection

🔴 Respect strict de client_max_body_size

Quand tu veux, je t’établis un plan d’implémentation par bloc avec ordre de priorité. Tu veux ça ?