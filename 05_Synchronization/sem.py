class Semaphore:
    # mutex is single key, but if we get more keys it become a counter--token
    token, waits = 1, ''

    # Aqure the key
    def P(self, tid):
        # try to get one key, and drop a token
        if self.token > 0:
            self.token -= 1
            return True
        else:
            # else we need to stay in queue,util other thread release the key and awake us
            self.waits = self.waits + tid
            return False
    # Release the key
    def V(self):
        # if someone wait for this key, we just awake a waiting thread
        if self.waits:
            self.waits = self.waits[1:]
        else:
            # else we just add a token
            self.token += 1

    @thread
    def t1(self):
        self.P('1')
        while '1' in self.waits: pass
        cs = True
        del cs
        self.V()

    @thread
    def t2(self):
        self.P('2')
        while '2' in self.waits: pass
        cs = True
        del cs
        self.V()

    @marker
    def mark_t1(self, state):
        if localvar(state, 't1', 'cs'): return 'blue'

    @marker
    def mark_t2(self, state):
        if localvar(state, 't2', 'cs'): return 'green'

    @marker
    def mark_both(self, state):
        if localvar(state, 't1', 'cs') and localvar(state, 't2', 'cs'):
            return 'red'
