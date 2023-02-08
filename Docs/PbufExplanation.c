err_t StateAutomaton(struct state *s,
                     struct tcp_pcb *pcb,
                     struct pbuf *p) {
  s->timeout = SERVER_TIMEOUT;
  for (;;) {
    uint8_t *c = (uint8_t *)p->payload;
    uint16_t i;
    err_t err;

    for (i = 0; i < p->len; ++i)
      if (ERR_OK != (err = s->function(s, pcb, c[i]))) //Wywolaj funkcje parsera lub colkiwek...
        return err;
      if (p->len == p->tot_len) 
        break;
      else
        p = p->next;
  }
  return ERR_OK;
}
