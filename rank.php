<?php
require_once 'config.php';
global $db, $genres, $countryCodes;

if (!isset($_GET['id']) || !is_numeric($_GET['id']))
    die(json_encode([
        'status' => false,
        'result' => 'invalid id'
    ]));

$result = [];
$id = $_GET['id'];
$country = isset($_GET['country']) && validateCountry($_GET['country']) ? $_GET['country'] : null;
$platform = isset($_GET['platform']) && validatePlatform($_GET['platform']) ? $_GET['platform'] : null;

$lastUpdateId = $db->query("SELECT `id` FROM `updates` WHERE `active` = true ORDER BY `id` DESC")->fetch_assoc()['id'];

if (!is_numeric($lastUpdateId))
    die(json_encode([
        'status' => false,
        'result' => 'internal error'
    ]));

$query = "SELECT `rank`, `apple-id`, `platform`, `type`, `genre`, `country` FROM `applications` WHERE `update_id` = $lastUpdateId AND `apple-id` = '$id' AND `active` = 1";

if ($country)
    $query .= " AND `country` = '$country'";

if ($platform)
    $query .= " AND `platform` = '$platform'";

$ranks = collect($db->query($query)->fetch_all(MYSQLI_ASSOC));

if ($ranks->isEmpty())
    die(json_encode([
        'status' => false,
        'result' => 'no result'
    ]));

$result = $ranks->map(function ($app) {
    return [
        'platform' => $app['platform'],
        'country' => $app['country'],
        'genre' => parseGenre($app['genre']),
        'type' => parseType($app['type']),
        'rank' => (int) $app['rank'],
        'category' => $app['genre'] == 36 ? 'all' : 'in-genre'
    ];
})->groupBy('country')->map(function ($groupedByCountry) {
    return $groupedByCountry->groupBy('category')->map(function ($groupedByCategory) {
        return $groupedByCategory->map(function ($item) {
            unset($item['category']);
            return $item;
        });
    });
});

echo json_encode([
    'status' => true,
    'result' => $result
]);

// Functions

function parseType($type) {
    if (strstr($type, 'Free'))
        return 'Free';

    else if (strstr($type, 'Paid'))
        return 'Paid';

    else if (strstr($type, 'Revenue'))
        return 'Grossing';
}

function parseGenre($genre) {
    global $genres;

    return $genres[$genre];
}

function validateCountry($country) {
    return preg_match('/^[A-Z]{2}$/', $country);
}

function validatePlatform($platform) {
    return in_array($platform, ['iphone', 'ipad', 'mac']);
}
