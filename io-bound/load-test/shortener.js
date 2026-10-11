// Uso:  docker run --rm -i --network host -e BASE_URL=http://localhost:8082 grafana/k6 run - < load-test/shortener.js
import http from 'k6/http';
import { check } from 'k6';

const BASE_URL = __ENV.BASE_URL || 'http://localhost:8082';
const LINKS = parseInt(__ENV.LINKS || '50', 10);

export const options = {
  stages: [
    { duration: '30s', target: 20 },
    { duration: '1m', target: 100 },
    { duration: '30s', target: 0 },
  ],
  thresholds: {
    http_req_failed: ['rate<0.05'],
    'http_req_duration{name:redirect}': ['p(95)<1000'],
  },
};

export function setup() {
  const codes = [];
  for (let i = 0; i < LINKS; i++) {
    const res = http.post(`${BASE_URL}/api/links`, JSON.stringify({ url: `https://example.com/${i}` }), {
      headers: { 'Content-Type': 'application/json' },
    });
    check(res, { 'link criado': (r) => r.status === 201 });
    codes.push(res.json('code'));
  }
  return { codes };
}

export default function ({ codes }) {
  const code = codes[Math.floor(Math.random() * codes.length)];
  const res = http.get(`${BASE_URL}/${code}`, { redirects: 0, tags: { name: 'redirect' } });
  check(res, { 'status 302': (r) => r.status === 302 });
}
