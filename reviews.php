<?php
header('Content-type: application/json');

$id = @$_GET['id'];
$country = @$_GET['country'];

if (!$id || !is_numeric($id))
    die(json_encode([
        'status' => false,
        'result' => 'bad id'
    ]));

if (!preg_match("/^[A-Za-z]{2}$/", $country))
    die(json_encode([
        'status' => false,
        'result' => 'bad country'
    ]));

$url = "https://itunes.apple.com/$country/rss/customerreviews/id=$id/sortBy=mostRecent/json";

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$response = json_decode(curl_exec($ch), true);

if (!$response)
    die(json_encode([
        'status' => false,
        'result' => 'no result'
    ]));

$result = [];

foreach ($response['feed']['entry'] as $review) {
    $result[] = [
        'title' => $review['title']['label'],
        'author' => $review['author']['name']['label'],
        'rate' => $review['im:rating']['label'],
        'date' => $review['updated']['label'],
        'version' => $review['im:version']['label'],
        'content' => $review['content']['label']
    ];
}

echo json_encode([
    'status' => true,
    'result' => $result
]);
