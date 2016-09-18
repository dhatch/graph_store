/**
 * status.h: A set of utilities for returning statuses from functions.
 */

#pragma once

#include "src/util/assert.h"

/**
 * Possible return status codes.
 */
enum class StatusCodes {
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
    DOES_NOT_EXIST
};

class Status {
public:
    Status() = delete;
    Status(StatusCode code): code(code) {};

    operator bool() const {
        return code == StatusCodes::SUCCESS;
    };

    StatusCode getCode() {
        return code;
    }
private
    StatusCode code;
}

template class StatusWith<typename T> : public Status {
public:
    /**
     * Create a successful status, with the result 'result'
     */
    StatusWith(T result) : Status(SUCCESS), result(result) {}

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
        invariant(code == StatusCode::SUCCESS);
        return result;
    }
private:
    union {
        T result;
        /**
         * Dummy data so that result can be 'null'.
         */
        char[sizeof(T)];
    };
}
