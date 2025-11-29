<?php
// Activer les erreurs (utile en développement)
ini_set('display_errors', 1);
error_reporting(E_ALL);

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    // 1️⃣ Récupération des données du formulaire
    $sex         = $_POST['sex'] ?? '';
    $firstName   = $_POST['FirstName'] ?? '';
    $name        = $_POST['Name'] ?? '';
    $birthDay    = $_POST['BirthDay'] ?? '';
    $status      = $_POST['status'] ?? '';
    $phone       = $_POST['phone'] ?? '';
    $email       = $_POST['email'] ?? '';
    $contract    = isset($_POST['contract']) ? 1 : 0;
    $information = $_POST['information'] ?? '';

    // 2️⃣ Vérifications minimales
    $errors = [];

    if (empty($email)) {
        $errors[] = "L'email est obligatoire.";
    }
    if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
        $errors[] = "L'email n'est pas valide.";
    }

    // 3️⃣ Gestion du fichier uploadé (CV)
    $cvFilePath = null;
    if (isset($_FILES['cv']) && $_FILES['cv']['error'] === UPLOAD_ERR_OK) {
        $uploadDir = __DIR__ . '/uploads/'; // dossier de destination
        if (!is_dir($uploadDir)) {
            mkdir($uploadDir, 0777, true);
        }

        // On nettoie le nom pour éviter les caractères problématiques
        $safeName = preg_replace('/[^a-zA-Z0-9_\-]/', '_', $name);

        // Récupérer l'extension du fichier original
        $originalName = $_FILES['cv']['name'];
        $extension = pathinfo($originalName, PATHINFO_EXTENSION);

        // Nouveau nom du fichier basé sur Name
        $newFileName = $safeName . '.' . $extension;
        $cvFilePath = $uploadDir . $newFileName;

        // Déplacer le fichier sous le nouveau nom
        if (!move_uploaded_file($_FILES['cv']['tmp_name'], $cvFilePath)) {
            $errors[] = "Le fichier n'a pas pu être uploadé.";
        }
    }

    // 4️⃣ Affichage ou traitement selon qu'il y a des erreurs
    if (!empty($errors)) {
        echo "<h3>Erreurs :</h3><ul>";
        foreach ($errors as $err) {
            echo "<li>$err</li>";
        }
        echo "</ul>";
        exit;
    }

    // 5️⃣ Exemple de traitement/affichage (tu peux remplacer par insertion en BDD)
    echo "<h3>Formulaire reçu avec succès :</h3>";
    echo "<p>Sex: $sex</p>";
    echo "<p>First Name: $firstName</p>";
    echo "<p>Name: $name</p>";
    echo "<p>BirthDay: $birthDay</p>";
    echo "<p>Status: $status</p>";
    echo "<p>Phone: $phone</p>";
    echo "<p>Email: $email</p>";
    echo "<p>Contract accepté: " . ($contract ? 'Oui' : 'Non') . "</p>";
    echo "<p>Informations: $information</p>";

    if ($cvFilePath) {
        echo "<p>CV enregistré dans : $cvFilePath</p>";
    } else {
        echo "<p>Aucun CV uploadé.</p>";
    }

} else {
    echo "Aucune donnée reçue.";
}
