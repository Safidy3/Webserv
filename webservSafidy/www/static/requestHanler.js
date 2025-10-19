// GET Request: Fetch data from the server
function sendGetRequest(url) {
	fetch(url, {
		method: 'GET',
		headers: {
			'Content-Type': 'application/json',
		},
	}).then(response => {
		if (!response.ok) {
			throw new Error('Network response was not ok');
		}
		return response.json();
	}).then(data => {console.log('GET Response Data:', data);
	}).catch(error => {console.error('Error during GET request:', error);
	});
}

// POST Request: Send data to the server
function sendPostRequest(url, data) {
	fetch(url, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json',
		},
		body: JSON.stringify(data),
	})
		.then(response => {
			if (!response.ok) {
				throw new Error('Network response was not ok');
			}
			return response.json();
		})
		.then(data => {
			console.log('POST Response Data:', data);
		})
		.catch(error => {
			console.error('Error during POST request:', error);
		});
}

// DELETE Request: Delete a resource on the server
function sendDeleteRequest(url) {
	fetch(url, {
		method: 'DELETE',
		headers: {
			'Content-Type': 'application/json',
		},
	})
		.then(response => {
			if (!response.ok) {
				throw new Error('Network response was not ok');
			}
			return response.json();
		})
		.then(data => {
			console.log('DELETE Response Data:', data);
		})
		.catch(error => {
			console.error('Error during DELETE request:', error);
		});
}

/*
// Example HTML to use the above JavaScript functions

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HTTP Requests Example</title>
    <script src="requests.js"></script>  <!-- Link to your JavaScript file -->
</head>
<body>

    <!-- GET Request -->
    <button onclick="sendGetRequest('/api/data')">Get Data</button>

    <!-- POST Request -->
    <button onclick="sendPostRequest('/api/submit', { name: 'John Doe', age: 30 })">Send Data (POST)</button>

    <!-- DELETE Request -->
    <button onclick="sendDeleteRequest('/api/delete-item/123')">Delete Item</button>

</body>
</html>

*/