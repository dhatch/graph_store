/**
 * status.h: A set of utilities for returning statuses from functions.
 */

#pragma once

#include <type_traits>

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
    INVALID,

    /**
     * No space remaining.
     */
    NO_SPACE,

    /**
     * This is the wrong partition
     */
    WRONG_PARTITION,

    PARTITION_FAIL
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

protected:
    StatusCode code;
};

template<typename T> class StatusWith : public Status {
public:
    /**
     * Create a successful status, with the result 'result'
     */
    template<typename = std::enable_if<std::is_copy_constructible<T>::value>>
    StatusWith(const T& result) : Status(StatusCode::SUCCESS), result(result), empty(false) {}

    template<typename std::enable_if<
        !std::is_reference<T>::value>::type* = nullptr>
    StatusWith(T&& result) : Status(StatusCode::SUCCESS), result(std::move(result)), empty(false) {}

    template<typename = std::enable_if<std::is_copy_constructible<T>::value>>
    StatusWith(const StatusWith& other) : Status(other.getCode()), result(other.result) {
        if (other.empty) {
            return;
        }

        empty = false;
    }

    template<typename = std::enable_if<std::is_copy_assignable<T>::value>>
    StatusWith& operator=(const StatusWith& other) {
        empty = other.empty;

        if (!empty) {
            result = other.result;
        }

        code = other.getCode();
        return *this;
    }

    template<typename = std::enable_if<std::is_move_constructible<T>::value>>
    StatusWith(StatusWith&& other) :
            Status(other.getCode()), result(std::move(other.result)) {
        if (other.empty) {
            return;
        }

        empty = false;
    }

    template<typename = std::enable_if<std::is_move_assignable<T>::value>>
    StatusWith& operator=(const StatusWith&& other) {
        empty = other.empty;

        if (!empty) {
            result = std::move(other.result);
        }

        code = other.getCode();
        return *this;
    }

    ~StatusWith() {
        if (!empty) {
            result.~T();
        }
    }

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
    bool empty = true;
};
