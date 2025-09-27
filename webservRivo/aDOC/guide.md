🔹 1. GET

But : récupérer une ressource statique (ex: index.html, image, CSS).

À faire :

Résoudre l’URI → trouver le fichier dans le root.

Vérifier que le fichier existe et est lisible.

Ouvrir et lire le contenu.

Retourner une réponse HTTP :

HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: <taille du fichier>

<contenu du fichier>


Si le fichier n’existe pas → 404 Not Found.

Si pas de droit → 403 Forbidden.

🔹 2. POST

    But : envoyer des données au serveur (formulaires, upload, CGI).

    Deux cas principaux :

    POST vers un CGI → tu envoies le body HTTP au stdin du CGI, et tu récupères la réponse du CGI (tu as déjà vu ça avec php-cgi).

    POST vers une route spéciale (ex: upload) → tu écris le body dans un fichier sur ton disque.

    Réponses typiques :

    201 Created si upload réussi,

    200 OK si c’est traité par CGI.

🔹 3. DELETE

But : supprimer une ressource.

À faire :

Résoudre l’URI → trouver le fichier.

Vérifier que le fichier existe et que tu as les droits.

Supprimer (unlink()).

Répondre :

200 OK ou 204 No Content si succès,

404 Not Found si le fichier n’existe pas,

403 Forbidden si pas le droit.

⚡ Points importants à gérer

Méthodes non supportées → répondre 405 Method Not Allowed + header Allow: GET, POST, DELETE.

Taille max des bodies (client_max_body_size dans le config) → répondre 413 Payload Too Large si dépassée.

CGI : gérer POST (body → stdin du CGI) et GET (query string → env CGI).

👉 Si tu veux, je peux te faire un schéma global du flux serveur :

réception requête HTTP,

parsing,

dispatch en fonction de la méthode (GET, POST, DELETE),

génération de la réponse HTTP.

Tu veux que je t’en prépare un ?