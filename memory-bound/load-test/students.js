// Uso:  docker run --rm -i --network host -e BASE_URL=http://localhost:8080 grafana/k6 run - < load-test/students.js
//
// A mistura é dominada por acessos pontuais (aluno, histórico e página), que custam
// pouca CPU, e a taxa de chegada é fixa (RATE req/s): o recurso que cresce com a
// escala é a RAM ocupada pelos dados, e a CPU fica ociosa.
// SCAN_RATIO (0 a 1) liga uma fração de varreduras (search/stats/ranking).
import http from 'k6/http';
import { check } from 'k6';

const BASE_URL = __ENV.BASE_URL || 'http://localhost:8080';
const RATE = parseInt(__ENV.RATE || '1000', 10);
const DURATION = __ENV.DURATION || '1m';
const SCAN_RATIO = parseFloat(__ENV.SCAN_RATIO || '0');

export const options = {
  scenarios: {
    students: {
      executor: 'ramping-arrival-rate',
      startRate: 0,
      timeUnit: '1s',
      preAllocatedVUs: 50,
      maxVUs: 500,
      stages: [
        { duration: '30s', target: RATE },
        { duration: DURATION, target: RATE },
        { duration: '30s', target: 0 },
      ],
    },
  },
  thresholds: {
    http_req_failed: ['rate<0.05'],
    'http_req_duration{name:student}': ['p(95)<100'],
    'http_req_duration{name:history}': ['p(95)<100'],
  },
};

export function setup() {
  const res = http.get(`${BASE_URL}/health`);
  check(res, { 'servidor no ar': (r) => r.status === 200 });
  return { total: res.json('students') };
}

function randomId(total) {
  return 1 + Math.floor(Math.random() * total);
}

export default function ({ total }) {
  if (Math.random() < SCAN_RATIO) {
    const scans = [
      ['search', `${BASE_URL}/students/search?name=zzz`],
      ['stats', `${BASE_URL}/students/stats`],
      ['ranking', `${BASE_URL}/students/ranking?top=10`],
    ];
    const [name, url] = scans[Math.floor(Math.random() * scans.length)];
    const res = http.get(url, { tags: { name } });
    check(res, { 'status 200': (r) => r.status === 200 });
    return;
  }

  const r = Math.random();
  let res;
  if (r < 0.5) {
    res = http.get(`${BASE_URL}/students/${randomId(total)}`, { tags: { name: 'student' } });
  } else if (r < 0.8) {
    res = http.get(`${BASE_URL}/students/${randomId(total)}/history`, { tags: { name: 'history' } });
  } else {
    const offset = Math.floor(Math.random() * Math.max(total - 20, 1));
    res = http.get(`${BASE_URL}/students?offset=${offset}&limit=20`, { tags: { name: 'page' } });
  }
  check(res, { 'status 200': (r) => r.status === 200 });
}
