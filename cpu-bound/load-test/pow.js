// Uso:  docker run --rm -i --network host -e BASE_URL=http://localhost:8080 -e DIFFICULTY=5 grafana/k6 run - < load-test/pow.js
import http from 'k6/http';
import { check } from 'k6';

const BASE_URL = __ENV.BASE_URL || 'http://localhost:8080';
const DIFFICULTY = parseInt(__ENV.DIFFICULTY || '5', 10);

export const options = {
  stages: [
    { duration: '30s', target: 5 },
    { duration: '1m', target: 20 },
    { duration: '30s', target: 0 },
  ],
  thresholds: {
    http_req_failed: ['rate<0.05'],
    http_req_duration: ['p(95)<5000'],
  },
};

export default function () {
  const payload = JSON.stringify({ data: `vu-${__VU}-iter-${__ITER}`, difficulty: DIFFICULTY });
  const res = http.post(`${BASE_URL}/api/pow`, payload, {
    headers: { 'Content-Type': 'application/json' },
    timeout: '60s',
  });
  check(res, {
    'status 200': (r) => r.status === 200,
    'hash válido': (r) => r.status === 200 && r.json('hash').startsWith('0'.repeat(DIFFICULTY)),
  });
}
