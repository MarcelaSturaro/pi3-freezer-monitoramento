const request = require('supertest');
const app = require('./server');


describe('API/api/temperatura', () => {
  it('deve retornar status 200 e um array (vazio) quando executado em ambiente de teste', async () => {
    const res = await request(app).get('/api/temperatura');
    expect(res.statusCode).toBe(200);
    expect(Array.isArray(res.body)).toBe(true);
  });
});
