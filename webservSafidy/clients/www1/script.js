document.getElementById('deleteBtn').addEventListener('click', function()
{
    let path = document.getElementById('filePath').value.trim();
    if (!path) {
        alert('Please enter a file path.');
        return;
    }

    fetch('/' + path, {
        method: 'DELETE'
    })
    .then(function(response) {
        if (response.status === 204) {
            document.getElementById('deleteResult').textContent = "File successfully deleted !";
        } else if (response.status === 404) {
            document.getElementById('deleteResult').textContent = "File not found (404).";
        } else if (response.status === 403) {
            document.getElementById('deleteResult').textContent = "Access denied (403).";
        } else {
            document.getElementById('deleteResult').textContent = "Error server (" + response.status + ").";
        }
    })
    .catch(function(err) {
        document.getElementById('deleteResult').textContent = "Error : " + err;
    });
});

