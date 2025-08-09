<?php
// For regular PHP (not php-cgi), we need to manually output headers
echo "Content-Type: text/html\r\n";
echo "\r\n";

echo "<!DOCTYPE html>\n";
echo "<html>\n<head>\n";
echo "    <title>PHP CGI Test</title>\n";
echo "    <style>\n";
echo "        body{font-family:Arial,sans-serif;margin:40px;}\n";
echo "        .info{background:#f0f0f0;padding:15px;margin:10px 0;border-radius:5px;}\n";
echo "        .form{background:#e8f4fd;padding:20px;margin:20px 0;border-radius:5px;border:1px solid #007cba;}\n";
echo "        input,button{padding:8px;margin:5px;border-radius:4px;border:1px solid #ccc;}\n";
echo "        button{background:#007cba;color:white;cursor:pointer;}\n";
echo "        button:hover{background:#005a87;}\n";
echo "        .test-links a{display:inline-block;margin:5px 10px 5px 0;padding:8px 12px;background:#28a745;color:white;text-decoration:none;border-radius:4px;}\n";
echo "    </style>\n";
echo "</head>\n<body>\n";
echo "    <h1>PHP CGI Script Working!</h1>\n";

// Display server information
echo "    <div class='info'>\n";
echo "        <h2>Server Information:</h2>\n";
echo "        <ul>\n";
echo "            <li><strong>Request Method:</strong> " . $_SERVER['REQUEST_METHOD'] . "</li>\n";
echo "            <li><strong>Query String:</strong> " . (isset($_SERVER['QUERY_STRING']) ? $_SERVER['QUERY_STRING'] : 'None') . "</li>\n";
echo "            <li><strong>Content Type:</strong> " . (isset($_SERVER['CONTENT_TYPE']) ? $_SERVER['CONTENT_TYPE'] : 'None') . "</li>\n";
echo "            <li><strong>Content Length:</strong> " . (isset($_SERVER['CONTENT_LENGTH']) ? $_SERVER['CONTENT_LENGTH'] : '0') . "</li>\n";
echo "            <li><strong>Script Name:</strong> " . (isset($_SERVER['SCRIPT_NAME']) ? $_SERVER['SCRIPT_NAME'] : 'Unknown') . "</li>\n";
echo "        </ul>\n";
echo "    </div>\n";

// Handle POST requests
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    echo "    <div class='info'>\n";
    echo "        <h2>POST Data (Raw Input):</h2>\n";
    $post_data = file_get_contents('php://input');
    echo "        <pre>" . htmlspecialchars($post_data) . "</pre>\n";
    echo "    </div>\n";
    
    // Try to parse URL-encoded data
    if (isset($_SERVER['CONTENT_TYPE']) && strpos($_SERVER['CONTENT_TYPE'], 'application/x-www-form-urlencoded') === 0) {
        parse_str($post_data, $parsed_data);
        if (!empty($parsed_data)) {
            echo "    <div class='info'>\n";
            echo "        <h2>Parsed POST Data:</h2>\n";
            echo "        <ul>\n";
            foreach ($parsed_data as $key => $value) {
                echo "            <li><strong>" . htmlspecialchars($key) . ":</strong> " . htmlspecialchars($value) . "</li>\n";
            }
            echo "        </ul>\n";
            
            // Special handling for name and age
            if (isset($parsed_data['name']) && isset($parsed_data['age'])) {
                echo "        <div style='background:#d4edda;padding:10px;margin:10px 0;border-radius:4px;color:#155724;'>\n";
                echo "            <strong>✅ POST Success!</strong> Hello <strong>" . htmlspecialchars($parsed_data['name']) . "</strong>, ";
                echo "you are <strong>" . htmlspecialchars($parsed_data['age']) . "</strong> years old!\n";
                echo "        </div>\n";
            }
            echo "    </div>\n";
        }
    }
}

// Handle GET parameters
if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
    parse_str($_SERVER['QUERY_STRING'], $get_params);
    if (!empty($get_params)) {
        echo "    <div class='info'>\n";
        echo "        <h2>GET Parameters:</h2>\n";
        echo "        <ul>\n";
        foreach ($get_params as $key => $value) {
            echo "            <li><strong>" . htmlspecialchars($key) . ":</strong> " . htmlspecialchars($value) . "</li>\n";
        }
        echo "        </ul>\n";
        
        // Special handling for name and age in GET
        if (isset($get_params['name']) && isset($get_params['age'])) {
            echo "        <div style='background:#d1ecf1;padding:10px;margin:10px 0;border-radius:4px;color:#0c5460;'>\n";
            echo "            <strong>✅ GET Success!</strong> Hello <strong>" . htmlspecialchars($get_params['name']) . "</strong>, ";
            echo "you are <strong>" . htmlspecialchars($get_params['age']) . "</strong> years old!\n";
            echo "        </div>\n";
        }
        echo "    </div>\n";
    }
}

// Display forms for testing
echo "    <div class='form'>\n";
echo "        <h2>🧪 Test GET Request (Form Input)</h2>\n";
echo "        <form method='GET' action='/test.php'>\n";
echo "            <div>\n";
echo "                <label for='get_name'>Name:</label><br>\n";
echo "                <input type='text' id='get_name' name='name' value='ayoub' required>\n";
echo "            </div>\n";
echo "            <div>\n";
echo "                <label for='get_age'>Age:</label><br>\n";
echo "                <input type='number' id='get_age' name='age' value='23' required>\n";
echo "            </div>\n";
echo "            <div>\n";
echo "                <button type='submit'>Send GET Request (Query String)</button>\n";
echo "            </div>\n";
echo "        </form>\n";
echo "        <p><small>📝 This will create a URL like: <code>/test.php?name=ayoub&age=23</code></small></p>\n";
echo "    </div>\n";

echo "    <div class='form'>\n";
echo "        <h2>🧪 Test POST Request (Form Data in Body)</h2>\n";
echo "        <form method='POST' action='/test.php'>\n";
echo "            <div>\n";
echo "                <label for='post_name'>Name:</label><br>\n";
echo "                <input type='text' id='post_name' name='name' value='Alice' required>\n";
echo "            </div>\n";
echo "            <div>\n";
echo "                <label for='post_age'>Age:</label><br>\n";
echo "                <input type='number' id='post_age' name='age' value='25' required>\n";
echo "            </div>\n";
echo "            <div>\n";
echo "                <button type='submit'>Send POST Request</button>\n";
echo "            </div>\n";
echo "        </form>\n";
echo "        <p><small>📝 This will send data in the request body: <code>name=Alice&age=25</code></small></p>\n";
echo "    </div>\n";

echo "    <div class='form'>\n";
echo "        <h2>🧪 Test GET Request (Query String)</h2>\n";
echo "        <div class='test-links'>\n";
echo "            <a href='/test.php?name=John&age=30'>Test: name=John&age=30</a>\n";
echo "            <a href='/test.php?name=Mary&age=22'>Test: name=Mary&age=22</a>\n";
echo "            <a href='/test.php?name=Bob&age=35&city=Paris'>Test: name=Bob&age=35&city=Paris</a>\n";
echo "        </div>\n";
echo "        <p><strong>Manual test:</strong> Try this URL: <code>/test.php?name=YourName&age=YourAge</code></p>\n";
echo "    </div>\n";

echo "    <p><a href='/cgi-test.html'>← Back to CGI Test Page</a></p>\n";
echo "</body>\n</html>\n";
?>
