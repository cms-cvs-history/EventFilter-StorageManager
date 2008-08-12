create or replace view view_last_five_runs as
SELECT "RUN_NUMBER",
       "START_TIME",
       "SETUPLABEL",
       "TOTAL_SIZE",
       "NEVTS",
       "NFILES",
       "RATE_AVG",
       "N_OPEN",
       "N_CLOSED",
       "N_SAFE0",
       "N_SAFE99",
       "N_DELETED"
  FROM ( SELECT TO_CHAR ( RUNNUMBER ) AS RUN_NUMBER,
		TO_CHAR ( MIN ( CTIME ),
			  'YYYY/MM/DD HH24:MI' ) AS START_TIME,
		TO_CHAR ( MIN ( SETUPLABEL ) ) AS SETUPLABEL,
		TO_CHAR ( ROUND ( SUM ( FILESIZE ) / 1073741824,
				  2 ) ) || ' GB' AS TOTAL_SIZE,
		TO_CHAR ( SUM ( NEVENTS ) ) AS NEVTS,
		TO_CHAR ( COUNT ( * ) ) AS NFILES,
		RATE_AVERAGE ( RUNNUMBER ) AS RATE_AVG,
		TO_CHAR ( COUNT ( * ) - COUNT ( FILESIZE ) ) AS N_OPEN,
		TO_CHAR ( COUNT ( FILESIZE ) ) AS N_CLOSED,
		TO_CHAR ( COUNT ( FILES_TRANS_COPIED.FILENAME ) ) AS N_SAFE0,
		TO_CHAR ( COUNT ( FILES_TRANS_CHECKED.FILENAME ) ) AS N_SAFE99,
		TO_CHAR ( COUNT ( FILES_DELETED.FILENAME ) ) AS N_DELETED
	   FROM ( SELECT RUNNUMBER,
			 FILENAME,
			 SETUPLABEL,
			 CTIME,
			 PRODUCER,
			 DENSE_RANK ( )
		    OVER ( ORDER BY RUNNUMBER DESC
			   NULLS LAST ) RUNNUMBER_rank
		    FROM FILES_CREATED RUNNUMBER_rank where PRODUCER = 'StorageManager') FILES_CREATED 
	   LEFT OUTER JOIN FILES_INJECTED
	     ON FILES_CREATED.FILENAME = FILES_INJECTED.FILENAME
	   LEFT OUTER JOIN FILES_TRANS_COPIED
	     ON FILES_CREATED.FILENAME = FILES_TRANS_COPIED.FILENAME
	   LEFT OUTER JOIN FILES_TRANS_CHECKED
	     ON FILES_CREATED.FILENAME = FILES_TRANS_CHECKED.FILENAME
	   LEFT OUTER JOIN FILES_DELETED
	     ON FILES_CREATED.FILENAME = FILES_DELETED.FILENAME
	  WHERE RUNNUMBER_rank <= 5 
	  GROUP BY RUNNUMBER )
 ORDER BY 1 DESC

grant select on view_last_five_runs to public;
