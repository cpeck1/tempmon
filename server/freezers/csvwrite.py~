import csv
import os
from freezers.models import Freezer

def write_update(Freezer):
    directory = "/home/cpeck1/workspace/tempmon3/tempmon/server/data/freezers/"

    '''
    Commit the freezer update to a CSV file. The file name is given by the 
    freezer number and the date portion of the last_update field for the 
    freezer, while the entry consists of the time, status and temperature 
    fields.
    '''
    update_datetime = str(Freezer.last_updated).split()
    update_date = update_datetime[0]
    update_time = update_datetime[1]

    fz_num = str(Freezer.id)
    fz_status = str(Freezer.status)
    fz_temp = str(Freezer.temperature)

    directory = directory + fz_num + "/"
    filename = "fz" + fz_num + "-" + update_date + ".csv"

    if not os.path.exists(directory):
        os.makedirs(directory)
    filepath = directory + filename

    try:
        with open(filepath): return
    except IOError:
        with open(filepath, "wb") as csvcreate:
            filecreator = csv.writer(csvcreate, dialect='excel')
            filecreator.writerow(["Time", "Status", "Temperature"])
    finally:
        with open(filepath, "a") as csvupdate:
            statuswriter = csv.writer(csvupdate, dialect='excel')
            statuswriter.writerow([update_time, fz_status, fz_temp])
