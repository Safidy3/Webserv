<?php
// Tu peux ajouter un traitement PHP en haut si besoin (ex: $_POST)
?>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Formulaire Bootstrap</title>
  <link rel="stylesheet" href="/bootstrap/sass/style.css" type="text/css" />
</head>
<body>
  <nav class="navbar navbar-expand-lg bg-body-tertiary">
  <div class="container-fluid">
      <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarTogglerDemo03" aria-controls="navbarTogglerDemo03" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
      </button>
      <a class="navbar-brand" href="#">WEBSERV</a>
      <div class="collapse navbar-collapse" id="navbarTogglerDemo03">
      <ul class="navbar-nav me-auto mb-2 mb-lg-0">
          <li class="nav-item">
            <a class="nav-link active" aria-current="page" href="/">Home</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" href="/cgi_bin/Contact.php">Contact us</a>
          </li>
          <li class="nav-item">
            <a class="nav-link" href="/delete/delete.html">Delete</a>
          </li>
      </ul>
      <form class="d-flex" role="search" action="/search" method="GET">
          <input class="form-control me-2" type="search" name="q" placeholder="Search" aria-label="Search"/>
          <button class="btn btn-outline-success" type="submit">Search</button>
      </form>
      </div>
  </div>
  </nav>
  <div class="container mt-4">
    <form action="/uploads" method="POST" enctype="multipart/form-data">
      
      <!-- État Civil -->
      <h5 class="mb-3">État Civil</h5>

      <div class="mb-3">
        <label class="form-label d-block">Sex</label>
        <div class="form-check form-check-inline">
          <input class="form-check-input" type="radio" name="sex" id="man" value="man">
          <label class="form-check-label" for="man">Man</label>
        </div>
        <div class="form-check form-check-inline">
          <input class="form-check-input" type="radio" name="sex" id="woman" value="woman">
          <label class="form-check-label" for="woman">Woman</label>
        </div>
      </div>

      <div class="row mb-3">
        <div class="col-sm-6">
          <label for="inputFirstName" class="form-label">First Name</label>
          <input type="text" class="form-control" name="FirstName" id="inputFirstName">
        </div>
        <div class="col-sm-6">
          <label for="inputName" class="form-label">Name</label>
          <input type="text" class="form-control" name="Name" id="inputName">
        </div>
      </div>

      <div class="row mb-3">
        <div class="col-sm-6">
          <label for="inputBirthDay" class="form-label">BirthDay</label>
          <input type="date" class="form-control" name="BirthDay" id="inputBirthDay">
        </div>
        <div class="col-sm-6">
          <label for="inputStatus" class="form-label">Family Status</label>
          <select class="form-select" name="status" id="inputStatus">
            <option value="s">Single</option>
            <option value="m">Married</option>
            <option value="d">Divorced</option>
          </select>
        </div>
      </div>

      <!-- Contact -->
      <h5 class="mt-4 mb-3">Contact</h5>
      <div class="row mb-3">
        <div class="col-sm-6">
          <label for="inputPhone" class="form-label">Phone Number</label>
          <input type="number" class="form-control" name="phone" id="inputPhone">
        </div>
        <div class="col-sm-6">
          <label for="inputEmail" class="form-label">Email</label>
          <input type="email" class="form-control" name="email" id="inputEmail" required>
        </div>
      </div>

      <!-- Attachment -->
      <h5 class="mt-4 mb-3">Attachment</h5>
      <div class="mb-3">
        <label for="inputCv" class="form-label">Browse CV here</label>
        <input type="file" class="form-control" name="cv" id="inputCv">
      </div>

      <!-- Contract -->
      <h5 class="mt-4 mb-3">Contract</h5>
      <div class="form-check mb-3">
        <input class="form-check-input" type="checkbox" name="contract" id="inputContract" checked>
        <label class="form-check-label" for="inputContract">I accept the terms of use</label>
      </div>

      <!-- Additional Information -->
      <h5 class="mt-4 mb-3">Additional information (optional)</h5>
      <div class="mb-3">
        <textarea class="form-control" name="information" id="inputInfo" rows="5" placeholder="Your message"></textarea>
      </div>

      <!-- Buttons -->
      <div class="d-flex gap-2 mt-3">
        <button type="submit" class="btn btn-primary">Post information</button>
        <button type="reset" class="btn btn-secondary">Cancel</button>
      </div>

    </form>
  </div>

<!-- JS Bootstrap (optionnel) -->
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
