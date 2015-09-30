def memoize(key):
    def _decorating_wrapper(func):
        cache_key = normalize_key(key, now) > now:
            return _cache[cache_key]

        ret = func(*args, **kwargs)
        _cache[cache_key] = ret
        _timestamps[cache_key] = now + VALIDITY_PERIOD
        return ret

    return _caching_wrapper
    return _decorating_wrapper


