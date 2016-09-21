/**
 * status.h: A set of utilities for returning statuses from functions.
 */

#pragma once

#include "util/assert.h"

/**
 * Possible return status codes.
 */
enum class StatusCode {
    /**
     * The operation was successful.
     */
    SUCCESS,

    /**
     * There was an unspecified error.
     */
    ERROR,

    /**
     * The operation resulted in no action being taken.
     */
    NO_ACTION,

    /**
     * The requested resource does not exist.
     */
    DOES_NOT_EXIST,

    /**
     * Invalid operation.
     */
    INVALID
};

class Status {
public:
    Status() = delete;
    Status(StatusCode code): code(code) {};

    operator bool() const {
        return code == StatusCode::SUCCESS;
    }

    bool operator==(const Status& other) const {
        return code == other.getCode();
    }

    StatusCode getCode() const {
        return code;
    }

private:
    StatusCode code;
};

template<typename T> class StatusWith : public Status {
public:
    /**
     * Create a successful status, with the result 'result'
     */
    StatusWith(T result) : Status(StatusCode::SUCCESS), result(result) {}

    /**
     * Create a non-successful status, with no result.
     */
    StatusWith(StatusCode code) : Status(code) {}

    /**
     * Access the contained result.
     *
     * Invalid if the status is not SUCCESS.
     */
    T& operator*() {
        invariant(getCode() == StatusCode::SUCCESS);
        return result;
    }

    T* operator->() {
        return &result;
    }
private:
    union {
        T result;
        /**
         * Dummy data so that result can be 'null'.
         */
        char __dummy[sizeof(T)];
    };
};
