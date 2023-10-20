from datetime import datetime

def time_difference_in_seconds(time1, time2):
    # Convert the time strings to datetime objects
    t1 = datetime.strptime(time1, '%H:%M:%S.%f')
    t2 = datetime.strptime(time2, '%H:%M:%S.%f')
    
    # Calculate the difference
    delta = t2 - t1
    
    # Return the difference in seconds
    return delta.total_seconds()

if __name__ == "__main__":
    import sys

    # Check if the correct number of arguments are provided
    if len(sys.argv) != 3:
        print("Usage: python script_name.py HH:MM:SS.ms HH:MM:SS.ms")
        sys.exit(1)

    time1 = sys.argv[1]
    time2 = sys.argv[2]

    print(f" time elapsed: {time_difference_in_seconds(time1, time2)} sec")
