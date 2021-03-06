<?php

/*
 * This file is part of the Predis package.
 *
 * (c) Daniele Alessandri <suppakilla@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace Predis\Commands;

/**
 * @link http://redis.io/commands/exists
 * @author Daniele Alessandri <suppakilla@gmail.com>
 */
class KeyExists extends Command
{
    /**
     * {@inheritdoc}
     */
    public function getId()
    {
        return 'EXISTS';
    }

    /**
     * {@inheritdoc}
     */
    public function parseResponse($data)
    {
        return (bool) $data;
    }
}
