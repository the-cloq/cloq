<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

session_start();
require 'db.php';

echo "Connected to database OK.<br>";

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $username = trim($_POST['username'] ?? '');
    $email = trim($_POST['email'] ?? '');
    $password = $_POST['password'] ?? '';

    if (!$username || !$email || !$password) {
        echo "Please fill all fields.<br>";
    } else {
        // For this test, store password as plain text (we will fix later)
        $stmt = $pdo->prepare("INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?)");
        $hashedPassword = password_hash($password, PASSWORD_DEFAULT);
        $result = $stmt->execute([$username, $email, $hashedPassword]);

        if ($result) {
            echo "User registered! ID: " . $pdo->lastInsertId();
        } else {
            echo "Registration failed.";
        }
    }
}
?>

<form method="post">
    <input name="username" placeholder="Username" required>
    <input type="email" name="email" placeholder="Email" required>
    <input type="password" name="password" placeholder="Password" required>
    <button type="submit">Register</button>
</form>
