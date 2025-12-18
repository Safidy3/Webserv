Partie CGI : Le flux non-bloquant détaillé
⏱️ Scénario : Client demande /cgi_bin/test.php
┌─────────────────────────────────────────────────────────────────┐
│ CLIENT 1 envoie GET /cgi_bin/test.php                          │
│ CLIENT 2 envoie GET /image.jpg (simultanément)                 │
└─────────────────────────────────────────────────────────────────┘
                            ↓
                    ┌───────────────┐
                    │ Event Loop    │
                    │ poll()        │
                    │ 1s timeout    │
                    └───────────────┘
                            ↓
        ┌───────────────────┴───────────────────┐
        ↓ Client 1 (CGI)              ↓ Client 2 (FILE)

🔄 Étape 1 : FORK() - Créer processus enfant (non-bloquant)

pid_t pid = fork();  // ← TRÈS RAPIDE ! (< 1ms)

if (pid == 0) {
    // CHILD PROCESS : exécute le script PHP
    execve("/usr/bin/php", ...);
} else {
    // PARENT (serveur) : continue IMMÉDIATEMENT
    // N'attend PAS que le PHP finisse !
    return;  // Revient à la boucle principale
}

Point clé : Le fork() crée un processus en parallèle ! Le serveur ne bloque pas.

📨 Étape 2 : Pipes - Communication entre parent et enfant
Le code crée 3 pipes :

┌─────────────────────────────────────────┐
│ CHILD PROCESS (PHP)                    │
│                                         │
│  stdout → [pipe_out] → parent lit      │
│  stderr → [pipe_err] → parent lit      │
│  stdin  ← [pipe_in]  ← parent écrit    │
└─────────────────────────────────────────┘

Exemple : PHP génère du HTML

<?php
// test.php
<?php
echo "Content-Type: text/html\r\n\r\n";
echo "<h1>Hello!</h1>";
?>

Cet output :

Content-Type: text/html

<h1>Hello!</h1>

Passe par le pipe → le parent le récolte

⏳ Étape 3 : Lecture non-bloquante avec poll()

// Parent POLL les pipes du CGI
struct pollfd fds[2];
fds[0].fd = fd_out;  // Pipe stdout
fds[1].fd = fd_err;  // Pipe stderr
fds[0].events = POLLIN;
fds[1].events = POLLIN;

// poll() attend les données SANS BLOQUER
int ret = poll(fds, 2, remaining_time);  // remaining_time = 9995ms si 5ms écoulées

if (ret > 0) {
    // Données disponibles !
    if (fds[0].revents & POLLIN) {
        read(fd_out, buffer, 4096);  // Lire non-bloquant
        out.append(buffer);
    }
}

Timeline réelle :

T=0ms   : fork() lancé
T=1ms   : PHP démarre
T=5ms   : PHP génère du HTML et écrit dans le pipe
T=5ms   : poll() se réveille immédiatement (données dispo)
T=6ms   : parent lit le HTML
T=7ms   : PHP se termine
T=8ms   : parent récolte exit status
T=9ms   : envoie réponse au client ✅

TOTAL = 9ms pour servir le client CGI !

Pendant ce temps : Le serveur traite d'autres clients !

🏆 Étape 4 : Récupérer exit status (waitpid WNOHANG)

// Parent récolte le status du processus
int status = 0;
pid_t result = waitpid(pid, &status, WNOHANG);
//                                 ↑
//                    N'ATTEND PAS ! (non-bloquant)

if (result == pid) {
    // Enfant a terminé !
    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);  // Ex: 0 = succès
    }
}

Comparaison :

// ❌ BLOQUANT (ancien)
waitpid(pid, &status, 0);  // Attend que l'enfant finisse...
// Le serveur est GELÉ ici !

// ✅ NON-BLOQUANT (actuel)
waitpid(pid, &status, WNOHANG);  // Retour immédiat
// WIFEXITED(status) ? déjà fini : pas encore fini

🔁 Scénario : 2 clients simultanés

T=0    : Client 1 → GET /cgi_bin/loop.php (10s CGI)
         Client 2 → GET /index.html (fichier statique)

T=0ms  : fork() pour Client 1, PHP démarre
T=1ms  : PHP en exécution
T=2ms  : poll() détecte Client 2 en lecture → read() + réponse immédiate ✅
T=5ms  : poll() check PHP → pas de données, timeout court
T=6ms  : poll() détecte Other clients
T=10s  : PHP Client 1 finit → waitpid() récolte output
T=10s+ : envoie réponse Client 1

RÉSULTAT : Client 2 servi en 2ms malgré Client 1 qui dure 10s !

⚡ Résumé des techniques non-bloquantes CGI
Opération	                Technique	Bénéfice
fork()	                    Création processus parallèle	            Parent ne bloque pas
poll(fds, ..., timeout)	    Attendre plusieurs events avec timeout	    Parent ne lit que si données prêtes
read(fd, buf, 4096)	        Lecture non-bloquante	                    Retour immédiat, même si 0 bytes
waitpid(..., WNOHANG)	    Récupérer status sans attendre	            Parent sait si enfant fini, sans bloquer

Résultat : ✅ Serveur traite plein de clients pendant que CGI s'exécute en arrière-plan !