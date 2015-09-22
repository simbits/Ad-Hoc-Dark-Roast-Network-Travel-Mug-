var lastError := "";

func setError(var err)
    lastError := err;
endfunc

func getError()
    return lastError;
endfunc
