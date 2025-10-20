#!/usr/bin/env bash
# Simple integration test: multipart POST to /uploads and verify CSV + file
set -e

BASE_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CSV="$BASE_DIR/servers/server_1/contacts.csv"
UPLOAD_DIR="$BASE_DIR/servers/server_1"
URL="http://127.0.0.1:8080/uploads"

TMPFILE="/tmp/test_upload_$$.txt"
echo "integration test content" > "$TMPFILE"

echo "Posting $TMPFILE to $URL"
HTTP_CODE=$(curl -s -o /tmp/upload_resp_body -w "%{http_code}" -F "FirstName=AutomIntegration" -F "Name=Tester" -F "email=auto@integration.test" -F "file=@$TMPFILE;filename=test_upload.txt" "$URL")

echo "HTTP status: $HTTP_CODE"
if [ "$HTTP_CODE" != "201" ] && [ "$HTTP_CODE" != "200" ]; then
  echo "Unexpected HTTP status $HTTP_CODE"
  exit 2
fi

# small wait for server to flush file
sleep 0.2

echo "Checking CSV $CSV for entry"
if ! grep -q "test_upload.txt" "$CSV" 2>/dev/null; then
  echo "CSV does not contain filename 'test_upload.txt'"
  echo "Last 20 lines of CSV:";
  tail -n 20 "$CSV" || true
  exit 3
fi

echo "CSV contains filename"

echo "Checking for uploaded file in $UPLOAD_DIR"
if [ -f "$UPLOAD_DIR/test_upload.txt" ]; then
  echo "Found uploaded file at $UPLOAD_DIR/test_upload.txt"
elif [ -f "$UPLOAD_DIR/uploads/test_upload.txt" ]; then
  echo "Found uploaded file at $UPLOAD_DIR/uploads/test_upload.txt"
else
  echo "Uploaded file not found in expected locations. Listing recent files:";
  ls -lt "$UPLOAD_DIR" | head -n 20
  ls -lt "$UPLOAD_DIR/uploads" 2>/dev/null | head -n 20 || true
  exit 4
fi

echo "Integration test passed"
rm -f "$TMPFILE"
exit 0
