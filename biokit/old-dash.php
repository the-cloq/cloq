<?php
session_start();

if (!isset($_SESSION['user_id'])) {
    header("Location: login.php");
    exit;
}

$username = $_GET['username'] ?? '';
if (!$username) {
    echo "Username missing.";
    exit;
}

$pdo = new PDO('mysql:host=sql308.epizy.com;dbname=epiz_26828963_biokit', 'epiz_26828963', '5qNUdTbDm5SgDVW');
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

// Fetch user by username
$userStmt = $pdo->prepare("SELECT * FROM users WHERE username = ?");
$userStmt->execute([$username]);
$user = $userStmt->fetch(PDO::FETCH_ASSOC);

if (!$user) {
    echo "User not found.";
    exit;
}

// Fetch page linked to user (may be empty)
$pageStmt = $pdo->prepare("SELECT * FROM pages WHERE user_id = ?");
$pageStmt->execute([$user['id']]);
$page = $pageStmt->fetch(PDO::FETCH_ASSOC) ?: [];

// Handle profile update form submission
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $bio = $_POST['bio'] ?? '';
    $theme = $_POST['theme'] ?? 'default';

    // Image upload handling
    if (!empty($_FILES['profile_image']['name'])) {
        $allowedTypes = ['image/jpeg', 'image/png', 'image/gif', 'image/webp'];
        $fileType = $_FILES['profile_image']['type'];
        $fileSize = $_FILES['profile_image']['size'];
        $fileTmpPath = $_FILES['profile_image']['tmp_name'];

        if (in_array($fileType, $allowedTypes) && $fileSize < 2 * 1024 * 1024) {
            $ext = pathinfo($_FILES['profile_image']['name'], PATHINFO_EXTENSION);
            $newFileName = uniqid('profile_', true) . '.' . $ext;
            $uploadDir = __DIR__ . '/uploads/';

            if (!is_dir($uploadDir)) {
                mkdir($uploadDir, 0755, true);
            }

            $destPath = $uploadDir . $newFileName;

            if (move_uploaded_file($fileTmpPath, $destPath)) {
                $profileImageUrl = 'uploads/' . $newFileName;
            } else {
                echo "Error uploading the image.";
                exit;
            }
        } else {
            echo "Invalid file type or file too large (max 2MB).";
            exit;
        }
    } else {
        $profileImageUrl = $page['profile_image_url'] ?? '';
    }

    if ($page) {
        // Update existing page
        $updatePage = $pdo->prepare("UPDATE pages SET bio = ?, theme = ?, profile_image_url = ? WHERE id = ?");
        $updatePage->execute([$bio, $theme, $profileImageUrl, $page['id']]);
    } else {
        // Insert new page
        $insertPage = $pdo->prepare("INSERT INTO pages (user_id, bio, theme, profile_image_url) VALUES (?, ?, ?, ?)");
        $insertPage->execute([$user['id'], $bio, $theme, $profileImageUrl]);

        // Refresh $page with new data
        $pageStmt->execute([$user['id']]);
        $page = $pageStmt->fetch(PDO::FETCH_ASSOC);
    }

    // Handle links update
    if (!empty($_POST['link'])) {
        $pdo->prepare("DELETE FROM links WHERE page_id = ?")->execute([$page['id']]);

        $linkData = $_POST['link'];
        foreach ($linkData as $order => $link) {
            if (!empty($link['label']) && !empty($link['url'])) {
                $stmt = $pdo->prepare("INSERT INTO links (page_id, label, url, icon, link_order) VALUES (?, ?, ?, ?, ?)");
                $stmt->execute([
                    $page['id'],
                    $link['label'],
                    $link['url'],
                    $link['icon'] ?? '',
                    $order
                ]);
            }
        }
    }

    // Redirect to avoid resubmission
    header("Location: dashboard.php?username=" . urlencode($username));
    exit;
}

// Fetch links
$linkStmt = $pdo->prepare("SELECT * FROM links WHERE page_id = ? ORDER BY link_order ASC");
$linkStmt->execute([$page['id'] ?? 0]);
$links = $linkStmt->fetchAll(PDO::FETCH_ASSOC);
?>

<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Edit Profile - <?= htmlspecialchars($username) ?></title>
  <link rel="stylesheet" href="css/style.css">
  <style>
    /* Your existing styles */
  </style>
</head>
<body>
  <h1>Edit BioKit Page: <?= htmlspecialchars($username) ?></h1>

  <form method="POST" enctype="multipart/form-data">
    <div class="input-group">
      <label>Bio</label>
      <textarea name="bio"><?= htmlspecialchars($page['bio'] ?? '') ?></textarea>
    </div>

    <div class="input-group">
      <label>Profile Image</label>
      <div id="dropzone" class="dropzone">
        <p>Drag & drop an image here, or click to upload</p>
        <input type="file" id="profileImageInput" name="profile_image" accept="image/*">
        <img id="imagePreview" src="<?= htmlspecialchars($page['profile_image_url'] ?? '') ?>" alt="Preview" style="<?= empty($page['profile_image_url']) ? 'display:none;' : '' ?>">
      </div>
    </div>

    <div class="input-group">
      <label>Theme</label>
      <select name="theme">
        <option value="default" <?= ($page['theme'] ?? 'default') === 'default' ? 'selected' : '' ?>>Default</option>
        <option value="dark" <?= ($page['theme'] ?? '') === 'dark' ? 'selected' : '' ?>>Dark</option>
        <option value="pastel" <?= ($page['theme'] ?? '') === 'pastel' ? 'selected' : '' ?>>Pastel</option>
      </select>
    </div>

    <h2>Links</h2>
    <div id="links">
      <?php foreach ($links as $i => $link): ?>
        <div class="link-block">
          <div class="input-group">
            <label>Label</label>
            <input type="text" name="link[<?= $i ?>][label]" value="<?= htmlspecialchars($link['label']) ?>">
          </div>
          <div class="input-group">
            <label>URL</label>
            <input type="text" name="link[<?= $i ?>][url]" value="<?= htmlspecialchars($link['url']) ?>">
          </div>
          <div class="input-group">
            <label>Icon</label>
            <input type="text" name="link[<?= $i ?>][icon]" value="<?= htmlspecialchars($link['icon']) ?>">
          </div>
        </div>
      <?php endforeach; ?>

      <div class="link-block">
        <div class="input-group">
          <label>Label</label>
          <input type="text" name="link[<?= count($links) ?>][label]" placeholder="New link label">
        </div>
        <div class="input-group">
          <label>URL</label>
          <input type="text" name="link[<?= count($links) ?>][url]" placeholder="https://example.com">
        </div>
        <div class="input-group">
          <label>Icon</label>
          <input type="text" name="link[<?= count($links) ?>][icon]" placeholder="🔗">
        </div>
      </div>
    </div>

    <button type="submit" class="submit-btn">Save Changes!</button>
  </form>

  <p><a href="/u/<?= urlencode($username) ?>">🔗 View public page</a></p>

  <script>
  const input = document.getElementById('profileImageInput');
  const preview = document.getElementById('imagePreview');

  input.addEventListener('change', function () {
    const file = input.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = function (e) {
      preview.src = e.target.result;
      preview.style.display = 'block';
    };
    reader.readAsDataURL(file);
  });
  </script>

</body>
</html>
