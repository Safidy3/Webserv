<?php
echo "<h1>CGI PHP Test</h1>";

// Afficher l'heure
echo "<p>Heure du serveur : " . date("Y-m-d H:i:s") . "</p>";

// Afficher les variables GET
if (!empty($_GET)) {
    echo "<h2>Variables GET :</h2><ul>";
    foreach ($_GET as $key => $value) {
        echo "<li>$key = $value</li>";
    }
    echo "</ul>";
}

// Afficher les variables POST
if (!empty($_POST)) {
    echo "<h2>Variables POST :</h2><ul>";
    foreach ($_POST as $key => $value) {
        echo "<li>$key = $value</li>";
    }
    echo "</ul>";
}

// Petit formulaire pour tester POST
echo '
<form method="POST">
    <input type="text" name="username" placeholder="Votre nom"/>
    <input type="submit" value="Envoyer"/>
</form>
';
?>
