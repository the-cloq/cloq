<?php
// u/username.php

// Connect to database
$pdo = new PDO('mysql:host=sql308.epizy.com;dbname=epiz_26828963_biokit', 'epiz_26828963', '5qNUdTbDm5SgDVW');
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

// Get username from URL
$username = $_GET['username'] ?? '';

// Fetch user and page
$stmt = $pdo->prepare("
    SELECT users.id as user_id, pages.*
    FROM users
    JOIN pages ON users.id = pages.user_id
    WHERE users.username = ?
    LIMIT 1
");
$stmt->execute([$username]);
$page = $stmt->fetch(PDO::FETCH_ASSOC);

if (!$page) {
    http_response_code(404);
    echo "Profile not found.";
    exit;
}

// Fetch links
$linkStmt = $pdo->prepare("
    SELECT * FROM links
    WHERE page_id = ?
    ORDER BY link_order ASC
");
$linkStmt->execute([$page['id']]);
$links = $linkStmt->fetchAll(PDO::FETCH_ASSOC);

// Read image transform properties
$zoom = floatval($page['zoom'] ?? 1);
$posX = intval($page['pos_x'] ?? 0);
$posY = intval($page['pos_y'] ?? 0);
?>
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title><?= htmlspecialchars($username) ?> | BioKit</title>
  <link rel="stylesheet" href="../css/style.css" />
</head>
<body class="theme-<?= htmlspecialchars($page['theme']) ?>">

<div class="profile-wrapper">
  <div class="profile-header">
    <?php if (!empty($page['profile_image_url'])): ?>
      <div class="profile-pic-wrapper" style="overflow: hidden; width: 120px; height: 120px; border-radius: 50%; position: relative;">
        <img
          src="../<?= htmlspecialchars($page['profile_image_url']) ?>"
          alt="Profile Photo"
          class="profile-pic"
          style="transform: translate(<?= $posX ?>px, <?= $posY ?>px) scale(<?= $zoom ?>);
                 position: absolute;
                 max-width: none;"
        />
      </div>
    <?php endif; ?>

    <h1 class="username"><?= htmlspecialchars($username) ?></h1>

    <?php if (!empty($page['bio'])): ?>
      <p class="bio"><?= nl2br(htmlspecialchars($page['bio'])) ?></p>
    <?php endif; ?>
  </div>
</div>

<div class="link-list">
  <?php foreach ($links as $link): ?>
    <a href="<?= htmlspecialchars($link['url']) ?>" class="link-button" target="_blank">
      <?= htmlspecialchars($link['icon']) ?> <?= htmlspecialchars($link['label']) ?>
    </a>
  <?php endforeach; ?>
</div>

<footer class="footer">
  <small>Powered by <strong>BioKit</strong></small>
</footer>

</body>
</html>
