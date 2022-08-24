<?php
set_time_limit(0);

include 'config.php';
global $countryCodes, $genres, $chartTypes, $notificationsLink, $globalHeaders, $db;

$responses = [];
$errors = [];

$insertUpdate = $db->query("INSERT INTO `updates` (`created_at`) VALUES (UNIX_TIMESTAMP())");
if (!$insertUpdate)
    die('internal error');

$updateId = $db->insert_id;

$links = makeUrls();
foreach ($links as $link) {
    $response = json_decode(execCurl($link['url']));

    if (!$response)
        $errors[] = [
            'url' => str_replace('https://itunes.apple.com/WebObjects/MZStoreServices.woa/ws/charts?limit=100&', '', $link['url']),
            'response' => $response
        ];
    else {
        if (empty($response->resultIds))
            continue;

        $values = '';
        $time = time();

        foreach ($response->resultIds as $index => $id) {
            $rank = $index + 1;

            $values .= "($updateId, '$id', $rank, '$link[platform]', '$link[type]', '$link[genre]', '$link[country]', '$time'), ";
        }

        // Remove last comma from value tuples and make query
        $query = "INSERT INTO `applications` (`update_id`, `apple-id`, `rank`, `platform`, `type`, `genre`, `country`, `created_at`) VALUES " . substr($values, 0 , -2);

        $insert = $db->query($query);

        if (!$insert)
            $responses[] = [
                'url' => str_replace('https://itunes.apple.com/WebObjects/MZStoreServices.woa/ws/charts?limit=100&', '', $link['url']),
                'query' => $query,
                'response' => $db->error
            ];
    }
}

$estimatedCount = (count($links) * 100) - 700; // China region doesn't have News category, so 7 requests will returns null
$negligibleCountOfMissingRanks = 0.1 * $estimatedCount;
$insertedCount = (int) $db->query("SELECT COUNT(`id`) as `count` FROM `applications` WHERE update_id = '$updateId'")->fetch_assoc()['count'];

// If less than X percent of data is incorrect, mark update as a valid update
$isValid = ($estimatedCount - $insertedCount) <= $negligibleCountOfMissingRanks;

if ($isValid) {
    $db->query("UPDATE `updates` SET `active` = 1, `count` = '$insertedCount' WHERE `id` = '$updateId'");
    $db->query("DELETE FROM `applications` WHERE update_id != '$updateId'");
}

$validation = [
    'update_id' => $updateId,
    'estimated_count' => $estimatedCount,
    'negligible_missing_count' => $negligibleCountOfMissingRanks,
    'inserted_count' => $insertedCount,
    'missing_count' => $estimatedCount - $insertedCount,
    'is_valid' => $isValid
];

file_put_contents('errors.txt', json_encode($errors));
file_put_contents('responses.txt', json_encode($responses));
file_put_contents('validation.txt', json_encode($validation));

if (!$isValid) exec("curl $notificationsLink");

echo json_encode([
    'validation' => $validation,
    'errors' => $errors,
    'queries' => $responses
]);

function execCurl($url) {
    global $globalHeaders;

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, $globalHeaders);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $result = curl_exec($ch);
    curl_close($ch);

    return $result;
}

function makeUrls() {
    global $genres, $countryCodes, $chartTypes;
    $genreKeys = array_keys($genres);

    $urls = [];

    foreach ($countryCodes as $country)
        foreach ($genreKeys as $genre)
            foreach ($chartTypes as $type) {
                $platform = stristr($type, 'ipad') ? 'ipad' : ($type === 'FreeMacAppsV2' ? 'mac' : 'iphone');
                $urls[] = [
                    'country' => $country,
                    'genre' => $genre,
                    'type' => $type,
                    'platform' => $platform,
                    'url' => buildUrl($country, $genre, $type)
                ];
            }

    return $urls;
}

function buildUrl($country, $genre, $type, $limit = 100) {
    $prefix = 'https://itunes.apple.com/WebObjects/MZStoreServices.woa/ws/charts?';

    // @TODO: make url manually instead of using http_build_query
    return
        $prefix .
        http_build_query([
            'limit' => $limit,
            'cc' => $country,
            'g' => $genre,
            'name' => $type
        ]);
}
