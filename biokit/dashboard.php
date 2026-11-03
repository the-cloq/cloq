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
  .dropzone {
    border: 2px dashed #aaa;
    border-radius: 10px;
    padding: 20px;
    text-align: center;
    cursor: pointer;
    transition: border-color 0.3s;
    position: relative;
  }

  .dropzone.dragover {
    border-color: #333;
    background-color: #f9f9f9;
  }

  .dropzone input[type="file"] {
    opacity: 0;
    position: absolute;
    width: 100%;
    height: 100%;
    top: 0;
    left: 0;
    cursor: pointer;
  }

  #imagePreview {
    margin-top: 10px;
    max-width: 100%;
    height: auto;
    display: block;
  }
  .image-mask {
  width: 150px;
  height: 150px;
  overflow: hidden;
  border-radius: 50%;
  position: relative;
  background-color: #f4f4f4;
  margin-top: 1em;
}

.image-mask img {
  position: absolute;
  top: 0;
  left: 0;
  cursor: grab;
  max-width: none;
  height: auto;
  user-select: none;
  pointer-events: none; /* stops click-drag conflicts */
}

.controls input[type="range"] {
  width: 100%;
  display: block;
}


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

  <div class="input-group">
  <label>Profile Image</label>

  <!-- Dropzone -->
  <div class="input-group">
  <label>Profile Image</label>

  <!-- Dropzone -->
  <div id="dropzone" class="dropzone">
    <p>Drag & drop an image here, or click to upload</p>
    <input type="file" id="profileImageInput" name="profile_image" accept="image/*">

    <!-- Preview Area -->
    <div id="imagePreviewWrapper" class="image-mask" style="<?= empty($page['profile_image_url']) ? 'display:none;' : '' ?>">
      <img id="imagePreview" 
           src="<?= htmlspecialchars($page['profile_image_url'] ?? '') ?>" 
           alt="Preview">
    </div>
  </div>

  <!-- Zoom Control -->
  <div id="imageControls" class="controls" style="margin-top: 1em; <?= empty($page['profile_image_url']) ? 'display:none;' : '' ?>">
    <label>Zoom</label>
    <input type="hidden" name="zoom" id="zoomInput" value="1">
    <input type="hidden" name="pos_x" id="posXInput" value="0">
    <input type="hidden" name="pos_y" id="posYInput" value="0">
    <input type="range" id="zoomSlider" min="1" max="3" step="0.1" value="1">
  </div>
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

  <p><a href="u/username.php?username=<?= urlencode($username) ?>">🔗 View public page</a>

  <script>
const input = document.getElementById('profileImageInput');
const previewWrapper = document.getElementById('imagePreviewWrapper');
const preview = document.getElementById('imagePreview');
const zoomSlider = document.getElementById('zoomSlider');
const controls = document.getElementById('imageControls');

let scale = 1;
let posX = 0, posY = 0;
let isDragging = false;
let startX = 0, startY = 0;
let initialX = 0, initialY = 0;

function updateTransform() {
  preview.style.transform = `translate(${posX}px, ${posY}px) scale(${scale})`;
}

zoomSlider.addEventListener('input', () => {
  scale = parseFloat(zoomSlider.value);
  updateTransform();
});

// Image dragging
previewWrapper.addEventListener('mousedown', (e) => {
  isDragging = true;
  startX = e.clientX;
  startY = e.clientY;
  initialX = posX;
  initialY = posY;
  preview.style.cursor = 'grabbing';
});

window.addEventListener('mousemove', (e) => {
  if (!isDragging) return;
  const dx = e.clientX - startX;
  const dy = e.clientY - startY;
  posX = initialX + dx;
  posY = initialY + dy;
  updateTransform();
});

window.addEventListener('mouseup', () => {
  isDragging = false;
  preview.style.cursor = 'grab';
});

// Image loading
input.addEventListener('change', (e) => {
  const file = e.target.files[0];
  if (!file) return;

  const reader = new FileReader();
  reader.onload = function(evt) {
    preview.src = evt.target.result;
    previewWrapper.style.display = 'block';
    controls.style.display = 'block';
    scale = 1;
    posX = 0;
    posY = 0;
    updateTransform();
    zoomSlider.value = 1;
  };
  reader.readAsDataURL(file);
});

function updateTransform() {
  preview.style.transform = `translate(${posX}px, ${posY}px) scale(${scale})`;

  // Update hidden inputs
  document.getElementById('zoomInput').value = scale;
  document.getElementById('posXInput').value = posX;
  document.getElementById('posYInput').value = posY;
}
</script>




</body>
</html>
